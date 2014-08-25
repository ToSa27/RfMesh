#include "RfMeshThreadNode.h"

bool RfMeshThreadNode::setup() {
//	db = new RfMeshDatabase();
	node = new RfMeshNode(1, 1024, 50, 50, 30);
	tc = new RfMeshTransceiverRfm70(RFM70_PIN_SCLK, RFM70_PIN_MOSI, RFM70_PIN_MISO, RFM70_PIN_RX_CSN, RFM70_PIN_RX_CE);
	node->addTransceiver(tc);
//	tc = new RfMeshTransceiverRfm70(RFM70_PIN_SCLK, RFM70_PIN_MOSI, RFM70_PIN_MISO, RFM70_PIN_TX_CSN, RFM70_PIN_TX_CE);
//	node->addTransceiver(tc);
	meshObservePTypes.push_back(RF_MESH_PTYPE_WHOAMI_REQUEST);
	meshObservePTypes.push_back(RF_MESH_PTYPE_DHCP_REQUEST);
	meshObservePTypes.push_back(RF_MESH_PTYPE_ANNOUNCE_REQUEST);
	meshObservePTypes.push_back(RF_MESH_PTYPE_FIRMWARE_REQUEST);
	meshObservePTypes.push_back(RF_MESH_PTYPE_CONFIG_REQUEST);
	node->addLogObserver(this);
	node->addMeshObserver(this);
	node->addAppObserver(this);
	if (!node->Init(RF_MESH_MASTER_ADDRESS)) {
		LOG4CXX_ERROR(logger, "...ERROR: node init failed\n");
		return false;
	}
	vector<uint16_t> nodes = RfMeshDatabase::getNodes();
	for (int i = 0; i < nodes.size(); i++) {
		if (RfMeshDatabase::needsUpdate(nodes[i])) {
			LOG4CXX_INFO(logger, "Node " + RfMeshUtils::u16tohex(nodes[i]) + " needs update.");
			if (RfMeshDatabase::nodeAutoUpdate(nodes[i]))
				SendAnnounceResponse(nodes[i]);
		} else {
			LOG4CXX_INFO(logger, "Node " + RfMeshUtils::u16tohex(nodes[i]) + " is up-to-date.");
		}
	}
	return true;
}

bool RfMeshThreadNode::loop() {
	node->Tick();
	if (!disp->mqSubscribe->empty()) {
		RfMeshThreadMessageSubscribePtr sp = (RfMeshThreadMessageSubscribePtr)disp->mqSubscribe->dequeue(*this);
		RfMeshAppSubscriber* subscriber = new RfMeshAppSubscriber(sp->getPType(), sp->getNode(), sp->getMessageQueue(), this);
		subscribers.push_back(subscriber);
		delete sp;
	}
	if (node->HasData()) {
//		std::cout << "hasData" << std::endl << std::flush;
		appMessage Message = node->Receive();
//		LOG4CXX_INFO(logger, "RfMeshThreadNode: push new message to mqRx");
		disp->mqRx->enqueue(*this, new RfMeshThreadMessageApp(Message));
		newAppMessage(Message);
	}
	if (!disp->mqTx->empty()) {
//		LOG4CXX_INFO(logger, "RfMeshThreadNode: found message in mqTx");
		RfMeshThreadMessageAppPtr mp = (RfMeshThreadMessageAppPtr)disp->mqTx->dequeue(*this);
		appMessage am = mp->getMessage();
		node->Send(am);
		delete mp;
	}
	if (!disp->mqFromUi->empty()) {
//		LOG4CXX_INFO(logger, "RfMeshThreadNode: found message in mqFromUi");
		RfMeshThreadMessageJsonPtr mp = (RfMeshThreadMessageJsonPtr)disp->mqFromUi->dequeue(*this);
		std::string json = mp->getJson();
		handleUiRequest(json);
		delete mp;
	}
	return true;
}

void RfMeshThreadNode::handleUiRequest(std::string json) {
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(json, root)) {
		printf("error parsing json\n");
		return;
	}
	std::string msgType = root["type"].asString();
	Json::Value data = root["data"];
	bool send = true;
	if (msgType == "dataLogging") {
		uint8_t logAddr = (uint8_t)atoi(data["dataLoggingAddress"].asCString());
		vector<string> data = RfMeshDatabase::getDataLogging(logAddr);
		for (int i = 0; i < data.size(); i++)
			disp->mqToUi->enqueue(*this, new RfMeshThreadMessage("Log:DataLogging", data[i]));
		return;
	}
	if (msgType == "update") {
		uint16_t na = (uint16_t)atoi(data["nodeAddress"].asCString());
		if (RfMeshDatabase::needsUpdate(na))
			SendAnnounceResponse(na);
		return;
	}
	appMessage am;
	am.source = RF_MESH_NOADDRESS;
	am.dest = (uint16_t)atoi(data["nodeAddress"].asCString());
	am.pid = 0;
	if (msgType == "getConfig") {
		am.ptype = RF_MESH_PTYPE_CONFIG_REQUEST;
		am.len = 2;
		am.data[0] = (uint8_t)atoi(data["configType"].asCString());
		am.data[1] = (uint8_t)atoi(data["configIndex"].asCString());
	} else if (msgType == "pushConfig") {
		SendAnnounceResponse(am.dest);
		send = false;
	} else if (msgType == "getMem") {
		am.ptype = RF_MESH_PTYPE_DATA_GET_REQUEST;
		am.len = 1;
		am.data[0] = (uint8_t)atoi(data["memAddress"].asCString());
	} else if (msgType == "setMem") {
		am.ptype = RF_MESH_PTYPE_DATA_SET;
		am.len = 3;
		am.data[0] = (uint8_t)atoi(data["memAddress"].asCString());
		uint16_t val = (uint16_t)atoi(data["memValue"].asCString());
		am.data[1] = (uint8_t)(val & 0x00FF);
		am.data[2] = (uint8_t)((val & 0xFF00) >> 8);
	} else if (msgType == "discover") {
		am.ptype = RF_MESH_PTYPE_DISCOVER_REQUEST;
		am.len = 0;
	} else if (msgType == "reset") {
		am.ptype = RF_MESH_PTYPE_RESET;
		am.len = 0;
	} else if (msgType == "checkForNewFirmware") {
		RfMeshDatabase::checkFirmwareDirectory();
	} else
		send = false;
	if (send)
		node->Send(am);
}

bool RfMeshThreadNode::newMeshMessage(meshMessage mm) {
//	LOG4CXX_INFO(logger, "newMeshMessage");
	switch (mm.ptype) {
		case RF_MESH_PTYPE_WHOAMI_REQUEST:
			SendWhoAmIResponse(mm);
			return true;
		case RF_MESH_PTYPE_DHCP_REQUEST:
			SendDhcpResponse(mm);
			return true;
		case RF_MESH_PTYPE_ANNOUNCE_REQUEST:
			SendAnnounceResponse(mm);
			return true;
		case RF_MESH_PTYPE_FIRMWARE_REQUEST:
			SendFirmwareResponse(mm);
			return true;
		case RF_MESH_PTYPE_CONFIG_REQUEST:
			SendConfigResponse(mm);
			return true;
	}
	return false;
}

// ToDo : get from somewhere else
#define DEFAULT_NODE_HW_TYPE	2
#define DEFAULT_NODE_HW_VERSION	1

void RfMeshThreadNode::SendWhoAmIResponse(meshMessage mm) {
	meshMessage mmr;
	mmr.tc = 255;
	mmr.to = RF_MESH_BROADCAST;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = RF_MESH_BROADCAST;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = RF_MESH_PTYPE_WHOAMI_RESPONSE;
	mmr.cost = 1;
	mmr.len = 2 + 6;
	mmr.data[0] = DEFAULT_NODE_HW_TYPE;
	mmr.data[1] = DEFAULT_NODE_HW_VERSION;
	uint8_t * mac = RfMeshDatabase::getNewMac();
	for (uint8_t i = 0; i < 6; i++)
		mmr.data[2 + i] = mac[i];
	node->Send(mmr);
}

void RfMeshThreadNode::SendDhcpResponse(meshMessage mm) {
	meshMessage mmr;
	DhcpRequest *req = (DhcpRequest *)mm.data;
	DhcpResponse *res = (DhcpResponse *)mmr.data;
	mmr.tc = 255;
	mmr.to = RF_MESH_BROADCAST;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = RF_MESH_BROADCAST;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = RF_MESH_PTYPE_DHCP_RESPONSE;
	mmr.cost = 1;
	mmr.len = 6 + 2; // ToDo : sizeof DhcpResponse;
	for (uint8_t i = 0; i < 6; i++) // ToDo : sizeof DhcpRequest.macAddress
		res->macAddress[i] = req->macAddress[i];
	uint16_t newAddress = RfMeshDatabase::getAddress(req->macAddress);
	res->newAddress = newAddress;
	node->Send(mmr);
}

void RfMeshThreadNode::SendAnnounceResponse(uint16_t address) {
	appMessage am;
	AnnounceResponse *res = (AnnounceResponse *)am.data;
	Firmware fw = RfMeshDatabase::getLatestFirmware(address);
	am.source = node->addr;
	am.dest = address;
	am.pid = 0;
	am.ptype = RF_MESH_PTYPE_ANNOUNCE_RESPONSE;
	am.len = 8; // ToDo : sizeof AnnounceResponse;
	res->fwVersion = fw.fwVersion;
	res->fwBlockCount = fw.fwBlockCount;
	res->fwCrc = fw.fwCrc;
	res->configCrc = RfMeshDatabase::getConfigCrc(address);
	node->Send(am);
}

void RfMeshThreadNode::SendAnnounceResponse(meshMessage mm) {
	meshMessage mmr;
	AnnounceRequest *req = (AnnounceRequest *)mm.data;
	AnnounceResponse *res = (AnnounceResponse *)mmr.data;
	RfMeshDatabase::editNodeDetails(mm.source, req->hwType, req->hwVersion, req->fwVersion, req->fwCrc);
	mmr.tc = mm.tc;
	mmr.to = mm.from;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = mm.source;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = RF_MESH_PTYPE_ANNOUNCE_RESPONSE;
	mmr.cost = 1;
	mmr.len = 8; // ToDo : sizeof AnnounceResponse;
	if (RfMeshDatabase::nodeAutoUpdate(mm.source) || (req->fwVersion == 0)) {
		Firmware fw = RfMeshDatabase::getLatestFirmware(req->hwType, req->hwVersion);
		res->fwVersion = fw.fwVersion;
		res->fwBlockCount = fw.fwBlockCount;
		res->fwCrc = fw.fwCrc;
	} else {
		res->fwVersion = req->fwVersion;
		res->fwBlockCount = 0;
		res->fwCrc = req->fwCrc;
	}
	res->configCrc = RfMeshDatabase::getConfigCrc(mm.source);
	node->Send(mmr);
}

void RfMeshThreadNode::SendFirmwareResponse(meshMessage mm) {
	meshMessage mmr;
	FirmwareRequest *req = (FirmwareRequest *)mm.data;
	FirmwareResponse *res = (FirmwareResponse *)mmr.data;
	mmr.tc = mm.tc;
	mmr.to = mm.from;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = mm.source;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = RF_MESH_PTYPE_FIRMWARE_RESPONSE;
	mmr.cost = 1;
	mmr.len = 18; // ToDo : sizeof FirmwareResponse;
	res->fwBlockIndex = req->fwBlockIndex;
	RfMeshDatabase::getFirmwareBlock(req->hwType, req->hwVersion, req->fwVersion, req->fwBlockIndex, res->fwBlock);
	node->Send(mmr);
}

void RfMeshThreadNode::SendConfigResponse(meshMessage mm) {
	meshMessage mmr;
	mmr.tc = mm.tc;
	mmr.to = mm.from;
	mmr.from = node->addr;
	mmr.source = node->addr;
	mmr.dest = mm.source;
	mmr.pid = (node->nextpid)++;
	mmr.ptype = RF_MESH_PTYPE_CONFIG_RESPONSE;
	mmr.cost = 1;
	mmr.len = RfMeshDatabase::getNodeConfig(mm.source, mm.data[0], mm.data[1], mmr.data);
	node->Send(mmr);
}

bool RfMeshThreadNode::newAppMessage(appMessage am) {
	bool ret = false;
	for (int i = 0; i < subscribers.size(); i++) {
		if (subscribers[i]->inAppScope(am)) {
			subscribers[i]->newAppMessage(am);
			ret = true;
		}
	}
	return ret;
}

void RfMeshThreadNode::newLogMessage(std::string msgType, void *msgData) {
	if (msgType == "meshMessageRx") {
		meshMessage mm = *(meshMessage *)msgData;
//		if (mm.ptype == RF_MESH_PTYPE_HOPACK)
//			return;
		std::string log = "NodeRx: tc:" + RfMeshUtils::u8tohex(mm.tc) + " source:" + RfMeshUtils::u16tohex(mm.source) + " from:" + RfMeshUtils::u16tohex(mm.from) + " to:" + RfMeshUtils::u16tohex(mm.to) + " dest:" + RfMeshUtils::u16tohex(mm.dest) + " pid:" + RfMeshUtils::u8tohex(mm.pid) + " ptype:" + RfMeshUtils::u8tohex(mm.ptype) + " cost:" + RfMeshUtils::u8tohex(mm.cost);
		LOG4CXX_INFO(logger, log);
		log = "        data[" + RfMeshUtils::u8tohex(mm.len) + "]:(";
		for (int j = 0; j < mm.len; j++) {
			if (j > 0)
				log += " ";
			log += RfMeshUtils::u8tohex(mm.data[j]);
		}
		log += ")";
		LOG4CXX_INFO(logger, log);
		disp->mqToUi->enqueue(*this, new RfMeshThreadMessage("Log:MeshMessageReceived", RfMeshUtils::MeshMessageToJson(mm)));
	} else if (msgType == "meshMessageTx") {
		meshMessage mm = *(meshMessage *)msgData;
//		if (mm.ptype == RF_MESH_PTYPE_HOPACK)
//			return;
		std::string log = "NodeTx: tc:" + RfMeshUtils::u8tohex(mm.tc) + " source:" + RfMeshUtils::u16tohex(mm.source) + " from:" + RfMeshUtils::u16tohex(mm.from) + " to:" + RfMeshUtils::u16tohex(mm.to) + " dest:" + RfMeshUtils::u16tohex(mm.dest) + " pid:" + RfMeshUtils::u8tohex(mm.pid) + " ptype:" + RfMeshUtils::u8tohex(mm.ptype) + " cost:" + RfMeshUtils::u8tohex(mm.cost);
		LOG4CXX_INFO(logger, log);
		log = "        data[" + RfMeshUtils::u8tohex(mm.len) + "]:(";
		for (int j = 0; j < mm.len; j++) {
			if (j > 0)
				log += " ";
			log += RfMeshUtils::u8tohex(mm.data[j]);
		}
		log += ")";
		LOG4CXX_INFO(logger, log);
		disp->mqToUi->enqueue(*this, new RfMeshThreadMessage("Log:MeshMessageTransmitted", RfMeshUtils::MeshMessageToJson(mm)));
	}
}
