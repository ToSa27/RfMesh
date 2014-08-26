#include "RfMeshNode.h"

#ifdef RF_MESH_NODETYPE_MASTER
LoggerPtr RfMeshNode::logger(Logger::getLogger("ToSa.Mesh"));
#endif


RfMeshNode::RfMeshNode(int transceiverCount, int RoutingTableCapacity, int AppMessageQueueCapacity, int MeshMessageBacklogCapacity, int MeshMessageTrackerCapacity) {
	nextpid = 0;
	tcsc = 0;
	tcs = new RfMeshTransceiverBase*[transceiverCount];
	rt = new RfMeshRoutingTable(RoutingTableCapacity);
	rxq = new RfMeshAppMessageQueue(AppMessageQueueCapacity);
	txb = new RfMeshMeshMessageBacklog(MeshMessageBacklogCapacity);
	rxt = new RfMeshMeshMessageTracker(MeshMessageTrackerCapacity);
#ifdef RF_MESH_NODETYPE_LEAF
	nextBankHoliday.year = 0;
	nextVacationStart.year = 0;
	nextVacationEnd.year = 0;
	tsValid = false;
#endif
}

RfMeshNode::~RfMeshNode() {
	delete rt;
	delete rxq;
	delete txb;
	delete rxt;
}

bool RfMeshNode::Init(uint16_t newaddr) {
	if (!SetAddress(newaddr))
		return false;
	while (addr == RF_MESH_NOADDRESS) {
		SendDhcpRequest();
		unsigned long resubmit = millis() + 15000; // ToDo : variable resubmit delay - here 15 seconds
		while ((addr == RF_MESH_NOADDRESS) && (millis() < resubmit)) {
			TickRx();
		}
	}
	return true;
}

bool RfMeshNode::SetAddress(uint16_t newAddr) {
	rt->is(newAddr);
	for (uint8_t i = 0; i < tcsc; i++)
		if (!tcs[i]->Init(newAddr))
			return false;
	addr = newAddr;
	return true;
}

void RfMeshNode::SetMacAddress(uint8_t *newMacAddr) {
	for (uint8_t i = 0; i < 6; i++)
		macAddress[i] = newMacAddr[i];
}

void RfMeshNode::Send(meshMessage mm) {
	if ((mm.ptype & 0x80) > 0)
		txb->add(mm);
	else
		txb->add(mm, 1);
	TickTx();
}

void RfMeshNode::SendWaitHopAck(meshMessage mm) {
	txb->add(mm);
	while (txb->has(mm.pid)) {
		Tick();
	}
}

bool RfMeshNode::Send(appMessage Message) {
	meshMessage mm = AppToMesh(Message);
	mm.pid = nextpid++;
	if ((mm.source == RF_MESH_BROADCAST) || (mm.source == RF_MESH_NOADDRESS))
		mm.source = addr;
	if (Message.dest != RF_MESH_BROADCAST && mm.to == RF_MESH_NOADDRESS)
		SendArpRequest(Message.dest);
	Send(mm);
	return true;
}

/*
#ifdef RF_MESH_NODETYPE_MASTER
appMessage RfMeshNode::getTimeMessage() {
	time_t raw;
	struct tm * local;
	time(&raw);
	local = localtime(&raw);
	appMessage am;
	am.source = RF_MESH_NOADDRESS;
	am.dest = RF_MESH_BROADCAST;
	am.pid = 0;
	am.ptype = RF_MESH_PTYPE_TIME_BROADCAST;
	am.len = 8; // = sizeof Timestamp;
	Timestamp *req = (Timestamp *)am.data;
	req->year = local->tm_year + 1900;
	req->month = local->tm_mon + 1;
	req->day = local->tm_mday;
	req->hour = local->tm_hour;
	req->minute = local->tm_min;
	req->second = local->tm_sec;
	req->weekday = local->tm_wday;
	return am;
}
#endif
*/

void RfMeshNode::Tick() {
	TickRx();
	TickTx();
}

void RfMeshNode::TickTx() {
	for (uint16_t i = 0; i < txb->capacity; i++) {
		if (txb->pending(i)) {
			meshMessage mm = txb->get(i);
#ifdef RF_MESH_NODETYPE_MASTER
//			LOG4CXX_INFO(logger, "RfMeshNode - TickTx: pending: " << RfMeshUtils::u16tos(i));		
//			LOG4CXX_INFO(logger, "  id  : " << RfMeshUtils::u16tos(i));		
//			LOG4CXX_INFO(logger, "  dest: " << RfMeshUtils::u16tos(mm.dest));		
//			LOG4CXX_INFO(logger, "  to  : " << RfMeshUtils::u16tos(mm.to));		
//			LOG4CXX_INFO(logger, "  tc  : " << RfMeshUtils::u8tos(mm.tc));		
#endif
			if (((mm.dest != RF_MESH_BROADCAST) && (mm.to == RF_MESH_NOADDRESS)) || ((mm.dest != RF_MESH_BROADCAST) && (mm.to != RF_MESH_BROADCAST) && (mm.tc == 255))) {
//			if ((mm.dest != RF_MESH_BROADCAST) && (mm.to != RF_MESH_BROADCAST) && (mm.tc == 255)) {
				if (rt->has(mm.dest)) {
					mm.tc = rt->getTc(mm.dest);
					mm.to = rt->getHop(mm.dest);
					txb->setTo(i, mm.to);
				}
				else {
#ifdef RF_MESH_NODETYPE_MASTER
//					LOG4CXX_INFO(logger, "RfMeshNode - TickTx: wait for route to: " << RfMeshUtils::u16tos(mm.dest));		
#endif
					txb->reset(i, false);
					continue;
				}
			}
#ifdef RF_MESH_NODETYPE_MASTER
			notifyLogObserver("meshMessageTx", &mm);
//			std::string log = "NodeTx: tc:" + RfMeshUtils::u8tohex(mm.tc) + " source:" + RfMeshUtils::u16tohex(mm.source) + " from:" + RfMeshUtils::u16tohex(mm.from) + " to:" + RfMeshUtils::u16tohex(mm.to) + " dest:" + RfMeshUtils::u16tohex(mm.dest) + " pid:" + RfMeshUtils::u8tohex(mm.pid) + " ptype:" + RfMeshUtils::u8tohex(mm.ptype) + " cost:" + RfMeshUtils::u8tohex(mm.cost);
//			LOG4CXX_INFO(logger, log);
//			log = "        data[" + RfMeshUtils::u8tohex(mm.len) + "]:(";
//			for (int j = 0; j < mm.len; j++) {
//				if (j > 0)
//					log += " ";
//				log += RfMeshUtils::u8tohex(mm.data[j]);
//			}
//			log += ")";
//			LOG4CXX_INFO(logger, log);
#endif
			if ((mm.dest == RF_MESH_BROADCAST) || (mm.to == RF_MESH_BROADCAST)) {
				for (uint8_t k = 0; k < tcsc; k++) {
//					LOG4CXX_INFO(logger, "Tx via transceiver " + (int)k);
					tcs[k]->Send(mm);
				}
			}
#ifdef RF_MESH_NODETYPE_MASTER
			else if (mm.tc >= tcsc) {
				LOG4CXX_ERROR(logger, "Unknown Transceiver Id: " << (int)mm.tc);
			}
#endif
			else {
//				LOG4CXX_INFO(logger, "Tx via transceiver " + (int)mm.tc);
				tcs[mm.tc]->Send(mm);
			}
			txb->reset(i, true);
		}
	}
}

void RfMeshNode::TickRx() {
	for (uint8_t tci = 0; tci < tcsc; tci++) {
		while (tcs[tci]->HasData()) {
			meshMessage mm = tcs[tci]->Receive();
#ifdef RF_MESH_NODETYPE_MASTER
			notifyLogObserver("meshMessageRx", &mm);
//		std::string log = "NodeRx: tc:" + RfMeshUtils::u8tohex(mm.tc) + " source:" + RfMeshUtils::u16tohex(mm.source) + " from:" + RfMeshUtils::u16tohex(mm.from) + " to:" + RfMeshUtils::u16tohex(mm.to) + " dest:" + RfMeshUtils::u16tohex(mm.dest) + " pid:" + RfMeshUtils::u8tohex(mm.pid) + " ptype:" + RfMeshUtils::u8tohex(mm.ptype) + " cost:" + RfMeshUtils::u8tohex(mm.cost);
//		LOG4CXX_INFO(logger, log);
//		log = "        data[" + RfMeshUtils::u8tohex(mm.len) + "]:(";
//		for (int j = 0; j < mm.len; j++) {
//			if (j > 0)
//				log += " ";
//			log += RfMeshUtils::u8tohex(mm.data[j]);
//		}
//		log += ")";
//		LOG4CXX_INFO(logger, log);
#endif
			if (mm.from == RF_MESH_BROADCAST) // from broadcast means invalid...
				continue;
			if (mm.source == addr) // don't reply to my own packages
				continue;
			if ((mm.ptype & 0x80) > 0) { // send hopack
				meshMessage mmh;
				mmh.tc = mm.tc;
				mmh.to = mm.from;
				mmh.from = addr;
				mmh.source = mm.dest;
				mmh.dest = mm.source;
				mmh.pid = nextpid++;
				mmh.ptype = RF_MESH_PTYPE_HOPACK;
				mmh.cost = 1;
				mmh.len = 1;
				mmh.data[0] = mm.pid;
				Send(mmh);
			}
			if (mm.source != RF_MESH_NOADDRESS) {
				if (rxt->get(mm.source) == mm.pid)
					continue; // already received before
				rxt->set(mm.source, mm.pid);
			}
			rt->add(mm.from, mm.from, tci, 1);
			rt->add(mm.source, mm.from, tci, mm.cost);
			uint8_t pos;
#ifdef RF_MESH_NODETYPE_MASTER
			if ((mm.dest == addr) || (mm.dest == RF_MESH_NOADDRESS) || (mm.dest == RF_MESH_BROADCAST))
				if (notifyMeshObserver(mm))
					continue;
#endif
			Timestamp *req;
			switch (mm.ptype) {
				case RF_MESH_PTYPE_HOPACK:
					txb->done(mm.data[0]);
					continue;
				case RF_MESH_PTYPE_ARP_REQUEST:
					for (uint8_t i = 0; i * 2 < mm.len; i++) {
						uint16_t raddr = (mm.data[i * 2 + 1] << 8) + mm.data[i * 2];
						if (raddr == addr)
							continue;
						rt->add(raddr, mm.from, tci, mm.cost - i + 1);
					}
					if (mm.source == addr)
						continue;
					if (mm.len + 2 > RF_MESH_MESH_MESSAGE_LEN_MAX)
						continue; // max hops reached
					if (mm.dest == addr) {
						meshMessage mmr;
						mmr.tc = mm.tc;
						mmr.to = mm.from;
						mmr.from = addr;
						mmr.source = addr;
						mmr.dest = mm.source;
						mmr.pid = nextpid++;
						mmr.ptype = RF_MESH_PTYPE_ARP_RESPONSE;
						mmr.cost = 1;
						mmr.len = mm.len + 2;
						for (uint8_t i = 0; i < mm.len; i++)
							mmr.data[i] = mm.data[i];
						mmr.data[mm.len] = (uint8_t)(addr & 0x00FF);
						mmr.data[mm.len + 1] = (uint8_t)((addr & 0xFF00) >> 8);
						Send(mmr);
						continue;
					}
					if (rt->has(mm.dest)) {
						meshMessage mmr;
						mmr.tc = mm.tc;
						mmr.to = mm.from;
						mmr.from = addr;
						mmr.source = addr;
						mmr.dest = mm.source;
						mmr.pid = nextpid++;
						mmr.ptype = RF_MESH_PTYPE_ARP_RESPONSE;
						mmr.cost = rt->getCost(mm.dest) + mm.cost;
						mmr.len = mm.len + 2;
						for (uint8_t i = 0; i < mm.len; i++)
							mmr.data[i] = mm.data[i];
						mmr.data[mm.len] = (uint8_t)(addr & 0x00FF);
						mmr.data[mm.len + 1] = (uint8_t)((addr & 0xFF00) >> 8);
						Send(mmr);
						continue;
					}
					Forward(mm, true, true);
					continue;
				case RF_MESH_PTYPE_ARP_RESPONSE:
					pos = 255;
					if (mm.dest == addr)
						pos = 0;
					else {
						for (uint8_t i = 0; i * 2 < mm.len; i++) {
							uint16_t raddr = (mm.data[i * 2 + 1] << 8) + mm.data[i * 2];
							if (raddr == addr) {
								pos = i + 1;
								break;
							}
						}
					}
					if (pos == 255)
						continue; // received arp response but not in route... error
					if (pos * 2 < mm.len) {
						uint16_t hop = (mm.data[pos * 2 + 1] << 8) + mm.data[pos * 2];
						for (uint8_t i = pos; i * 2 < mm.len; i++) {
							uint16_t raddr = (mm.data[i * 2 + 1] << 8) + mm.data[i * 2];
							rt->add(raddr, hop, tci, i - pos + 1);
						}
					}
					if (mm.dest == addr)
						continue;
					if (Forward(mm, false, false))
						continue;
					// error
					continue;
				case RF_MESH_PTYPE_ARP_FAILURE:
					// ToDo
					break;
#ifndef RF_MESH_NODETYPE_MASTER
				case RF_MESH_PTYPE_DHCP_REQUEST: // if this is a dhcp server this will never be reached
					if (Forward(mm, true, false))
						continue;
					// error
					continue;
#endif
				case RF_MESH_PTYPE_DHCP_RESPONSE:
					if (addr == RF_MESH_NOADDRESS) {
						bool macMatch = true;
						for (uint8_t m = 0; m < 6; m++)
							if (mm.data[m] != macAddress[m]) {
								macMatch = false;
								break;
							}
						if (macMatch) {
							uint16_t newAddress = (mm.data[6 + 1] << 8) + mm.data[6];
							SetAddress(newAddress);
							continue;
						}
					}
					if (Forward(mm, true, false))
						continue;
					// error
					continue;				
				case RF_MESH_PTYPE_ACK:
					if (mm.dest == addr) {
						// ToDo
						continue;
					}
					if (Forward(mm, false, false))
						continue;
					// ToDo : probably want to send information about unknown route back to sender
					continue;
				case RF_MESH_PTYPE_NACK:
					// ToDo
					continue;
				case RF_MESH_PTYPE_PING:
					if (mm.dest == addr) {
						meshMessage mmr;
						mmr.tc = mm.tc;
						mmr.to = mm.from;
						mmr.from = addr;
						mmr.source = addr;
						mmr.dest = mm.source;
						mmr.pid = nextpid++;
						mmr.ptype = RF_MESH_PTYPE_PONG;
						mmr.cost = 1;
						mmr.len = 1;
						mmr.data[0] = mm.pid;
						Send(mmr);
						continue;
					}
					if (Forward(mm, false, false))
						continue;
					// ToDo : probably want to send information about unknown route back to sender
					continue;
				case RF_MESH_PTYPE_TIME_REQUEST:
#ifndef RF_MESH_NODETYPE_MASTER
					Forward(mm, true, false);
#endif
					continue;
				case RF_MESH_PTYPE_TIME_BROADCAST:
#ifdef RF_MESH_NODETYPE_LEAF
					tsMillis = millis();
					req = (Timestamp *)mm.data;
					tsLast.year = req->year;
					tsLast.month = req->month;
					tsLast.day = req->day;
					tsLast.hour = req->hour;
					tsLast.minute = req->minute;
					tsLast.second = req->second;
					tsLast.weekday = req->weekday;
					tsValid = true;
#endif
					Forward(mm, true, false);
					continue;
				case RF_MESH_PTYPE_HOLIDAY_BROADCAST:
#ifdef RF_MESH_NODETYPE_LEAF
					nextBankHoliday.year = (mm.data[1] << 8) + mm.data[0];
					nextBankHoliday.month = mm.data[2];
					nextBankHoliday.day = mm.data[3];
					nextVacationStart.year = (mm.data[5] << 8) + mm.data[4];
					nextVacationStart.month = mm.data[6];
					nextVacationStart.day = mm.data[7];
					nextVacationEnd.year = (mm.data[9] << 8) + mm.data[8];
					nextVacationEnd.month = mm.data[10];
					nextVacationEnd.day = mm.data[11];
#endif
					Forward(mm, true, false);
					continue;
#ifndef RF_MESH_NODETYPE_MASTER
				case RF_MESH_PTYPE_ANNOUNCE_REQUEST: // if this is a firmware server this will never be reached
					Forward(mm, false, false);
					continue;
				case RF_MESH_PTYPE_FIRMWARE_REQUEST: // if this is a firmware server this will never be reached
					Forward(mm, false, false);
					continue;
				case RF_MESH_PTYPE_DATASYNC:
					rxq->enqueue(MeshToApp(mm));
					if (mm.dest == addr) {
						meshMessage mmr;
						mmr.tc = mm.tc;
						mmr.to = mm.from;
						mmr.from = addr;
						mmr.source = addr;
						mmr.dest = mm.source;
						mmr.pid = nextpid++;
						mmr.ptype = RF_MESH_PTYPE_ACK;
						mmr.cost = 1;
						mmr.len = 1;
						mmr.data[0] = mm.pid;
						Send(mmr);
						continue;
					} else {
						Forward(mm, true, false);
					}
					continue;
#endif
				default:
					if (mm.dest == addr) {
						if ((mm.ptype & 0x40) > 0) {
							meshMessage mmr;
							mmr.tc = mm.tc;
							mmr.to = mm.from;
							mmr.from = addr;
							mmr.source = addr;
							mmr.dest = mm.source;
							mmr.pid = nextpid++;
							mmr.ptype = RF_MESH_PTYPE_ACK;
							mmr.cost = 1;
							mmr.len = 1;
							mmr.data[0] = mm.pid;
							Send(mmr);
						}
						appMessage am = MeshToApp(mm);
#ifdef RF_MESH_NODETYPE_MASTER
						if (notifyAppObserver(am))
							continue;
#endif
						rxq->enqueue(am);
						continue;
					}
//#ifdef RF_MESH_NODETYPE_MASTER
//				if ((mm.dest == RF_MESH_BROADCAST) && (std::find(answerBroadcastTypes.begin(), answerBroadcastTypes.end(), mm.ptype) != answerBroadcastTypes.end())) {
//					rxq->enqueue(MeshToApp(mm));
//					continue;
//				}
//#endif
					if (Forward(mm, false, false))
						continue;
					// no known route to dest
					continue;
			}
		}
	}
}

bool RfMeshNode::Forward(meshMessage mm, bool isBroadcast, bool addAddressToData) {
	uint8_t tc = 255;
	uint16_t to = RF_MESH_BROADCAST;
	if (!isBroadcast) {
		if (!rt->has(mm.dest))
			return false;
		tc = rt->getTc(mm.dest);
		to = rt->getHop(mm.dest);
	}
	meshMessage mmr;
	mmr.tc = tc;
	mmr.to = to;
	mmr.from = addr;
	mmr.source = mm.source;
	mmr.dest = mm.dest;
	mmr.pid = mm.pid;
	mmr.ptype = mm.ptype;
	mmr.cost = mm.cost + 1;
	if (!addAddressToData)
		mmr.len = mm.len;
	else if (mm.len + 2 <= RF_MESH_MESH_MESSAGE_LEN_MAX)
		mmr.len = mm.len + 2;
	else
		return false; // max hops reached
	for (uint8_t i = 0; i < mm.len; i++)
		mmr.data[i] = mm.data[i];
	if (addAddressToData) {
		mmr.data[mm.len] = (uint8_t)(addr & 0x00FF);
		mmr.data[mm.len + 1] = (uint8_t)((addr & 0xFF00) >> 8);
	}
	Send(mmr);
	return true;
}

bool RfMeshNode::HasData() {
	TickRx();
	return !rxq->empty();
}

appMessage RfMeshNode::Receive() {
	appMessage am;
	if (HasData())
		am = rxq->dequeue();
	return am;
}

appMessage RfMeshNode::MeshToApp(meshMessage mm) {
	appMessage am;
	am.source = mm.source;
	am.dest = mm.dest;
	am.pid = mm.pid;
	am.ptype = mm.ptype;
	am.len = mm.len;
	for (uint8_t i = 0; i < mm.len; i++)
		am.data[i] = mm.data[i];
	return am;
}

meshMessage RfMeshNode::AppToMesh(appMessage am) {
	meshMessage mm;
	mm.tc = 255;
	if (am.dest == RF_MESH_BROADCAST)
		mm.to = RF_MESH_BROADCAST;
	else
		mm.to = rt->getHop(am.dest);
	mm.from = addr;
	if (am.source == RF_MESH_NOADDRESS)
		mm.source = addr;
	else
		mm.source = am.source;
	mm.dest = am.dest;
	mm.pid = am.pid;
	mm.ptype = am.ptype;
	mm.cost = 1;
	mm.len = am.len;
	for (uint8_t i = 0; i < am.len; i++)
		mm.data[i] = am.data[i];
	return mm;
}

void RfMeshNode::SendArpRequest(uint16_t dest) {
	meshMessage mm;
	mm.tc = 255;
	mm.to = RF_MESH_BROADCAST;
	mm.from = addr;
	mm.source = addr;
	mm.dest = dest;
	mm.pid = nextpid++;
	mm.ptype = RF_MESH_PTYPE_ARP_REQUEST;
	mm.cost = 1;
	mm.len = 0;
	Send(mm);
}

void RfMeshNode::SendDhcpRequest() {
	meshMessage mm;
	mm.tc = 255;
	mm.to = RF_MESH_BROADCAST;
	mm.from = addr;
	mm.source = addr;
	mm.dest = RF_MESH_NOADDRESS;
	mm.pid = nextpid++;
	mm.ptype = RF_MESH_PTYPE_DHCP_REQUEST;
	mm.cost = 1;
	mm.len = 6;
	for (uint8_t i = 0; i < 6; i++)
		mm.data[i] = macAddress[i];
	Send(mm);
}

void RfMeshNode::SendPing(uint16_t dest) {
	appMessage am;
	am.source = addr;
	am.dest = dest;
	am.pid = 0;
	am.ptype = RF_MESH_PTYPE_PING;
	am.len = 0;
	Send(am);
}

void RfMeshNode::SendHeartbeat(uint16_t dest) {
	appMessage am;
	am.source = addr;
	am.dest = dest;
	am.pid = 0;
	am.ptype = RF_MESH_PTYPE_HEARTBEAT;
	am.len = 0;
	Send(am);
}

void RfMeshNode::addTransceiver(RfMeshTransceiverBase *tcvr) {
	tcvr->setIndex(tcsc);
	tcs[tcsc] = tcvr;
	tcsc++;
}

#ifdef RF_MESH_NODETYPE_LEAF
void RfMeshNode::refreshTime() {
	uint32_t currentMillis = millis();
	uint32_t elapsed = currentMillis - tsMillis;
	uint16_t level;
	elapsed = elapsed / 1000;
	if (elapsed == 0)
		return;
	tsMillis = currentMillis;
	level = elapsed % 60;
	if (tsLast.second + level < 60) {
		tsLast.second += level;
		elapsed -= level;
	}
	else {
		tsLast.second = tsLast.second + level - 60;
		elapsed = elapsed - level + 60;
	}
	elapsed = elapsed / 60;
	if (elapsed == 0)
		return;
	level = elapsed % 60;
	if (tsLast.minute + level < 60) {
		tsLast.minute += level;
		elapsed -= level;
	}
	else {
		tsLast.minute = tsLast.minute + level - 60;
		elapsed = elapsed - level + 60;
	}
	elapsed = elapsed / 60;
	if (elapsed == 0)
		return;
	level = elapsed % 24;
	if (tsLast.hour + level < 24) {
		tsLast.hour += level;
		elapsed -= level;
	}
	else {
		tsLast.hour = tsLast.hour + level - 24;
		elapsed = elapsed - level + 24;
	}
	elapsed = elapsed / 24;
	if (elapsed == 0)
		return;
	level = elapsed % 7;
	if (tsLast.weekday + level < 7)
		tsLast.weekday += level;
	else
		tsLast.weekday = tsLast.weekday + level - 7;
}
#endif

#ifdef RF_MESH_NODETYPE_MASTER
//void RfMeshNode::answerBroadcastType(uint8_t type) {
//	answerBroadcastTypes.push_back(type);
//}

void RfMeshNode::addLogObserver(ILogMessageObserver* obj) {
	logObservers.push_back(obj);
}

void RfMeshNode::addMeshObserver(IMeshMessageObserver* obj) {
	meshObservers.push_back(obj);
}

void RfMeshNode::addAppObserver(IAppMessageObserver* obj) {
	appObservers.push_back(obj);
}

void RfMeshNode::notifyLogObserver(std::string msgType, void *msgData) {
//	LOG4CXX_INFO(logger, "notifyLogObserver");
	for (int i = 0; i < logObservers.size(); i++)
		logObservers[i]->newLogMessage(msgType, msgData);
}

bool RfMeshNode::notifyMeshObserver(meshMessage mm) {
//	LOG4CXX_INFO(logger, "notifyMeshObserver");
	bool res = false;
	for (int i = 0; i < meshObservers.size(); i++)
		if (meshObservers[i]->inMeshScope(mm))
			if (meshObservers[i]->newMeshMessage(mm))
				res = true;
	return res;
}

bool RfMeshNode::notifyAppObserver(appMessage am) {
	bool res = false;
	for (int i = 0; i < appObservers.size(); i++)
		if (appObservers[i]->inAppScope(am))
			if (appObservers[i]->newAppMessage(am))
				res = true;
	return res;
}

#endif
