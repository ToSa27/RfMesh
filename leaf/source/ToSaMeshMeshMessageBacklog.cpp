#include "RfMeshMeshMessageBacklog.h"

#ifdef RF_MESH_NODETYPE_MASTER
LoggerPtr RfMeshMeshMessageBacklog::logger(Logger::getLogger("ToSa.Mesh"));
#endif

RfMeshMeshMessageBacklog::RfMeshMeshMessageBacklog(int capacity) {
	this->capacity = capacity;
	backlog = new meshMessageBacklogEntry[capacity];
	for (uint16_t i = 0; i < capacity; i++)
		backlog[i].redo = 0;
}

RfMeshMeshMessageBacklog::~RfMeshMeshMessageBacklog() {
	delete [] backlog;
}

void RfMeshMeshMessageBacklog::add(meshMessage mm) {
	add(mm, RF_MESH_HOP_RETRY);
}

void RfMeshMeshMessageBacklog::add(meshMessage mm, uint8_t redo) {
	for (uint16_t i = 0; i < capacity; i++)
		if (backlog[i].redo == 0) {
			if (redo == 1)
				backlog[i].redo = redo;
			else
				backlog[i].redo = redo | 0x80;
			backlog[i].next = millis(); // direct first time submission
			backlog[i].tc = mm.tc;
			backlog[i].to = mm.to;
			backlog[i].from = mm.from;
			backlog[i].source = mm.source;
			backlog[i].dest = mm.dest;
			backlog[i].pid = mm.pid;
			backlog[i].ptype = mm.ptype;
			backlog[i].cost = mm.cost;
			backlog[i].len = mm.len;
			for (uint8_t j = 0; j < backlog[i].len; j++)
				backlog[i].data[j] = mm.data[j];
#ifdef RF_MESH_NODETYPE_MASTER
//			LOG4CXX_INFO(logger, "RfMeshMeshMessageBacklog entry added at position " << RfMeshUtils::u16tos(i) << " with redo " << RfMeshUtils::u8tos(backlog[i].redo));		
#endif
			return;
		}
	// ToDo - handle backlog overflow
}

bool RfMeshMeshMessageBacklog::pending(uint16_t i) {
	return ((backlog[i].redo > 0) && (backlog[i].next < millis()));
}

void RfMeshMeshMessageBacklog::reset(uint16_t i, bool submitted) {
	if (backlog[i].redo > 0) {
		unsigned long pause = 1000 / (2 * (backlog[i].redo & 0x7F));
		if (submitted)
			backlog[i].redo = backlog[i].redo - 1;
		// ToDo : random delay
		backlog[i].next = millis() + pause;
#ifdef RF_MESH_NODETYPE_MASTER
//		LOG4CXX_INFO(logger, "RfMeshMeshMessageBacklog - reset: " << RfMeshUtils::u16tos(i) << " now has redo: " << RfMeshUtils::u8tos(backlog[i].redo));		
#endif
	}
}

meshMessage RfMeshMeshMessageBacklog::get(uint16_t i) {
	meshMessage mm;
//	copy(backlog[i].message, mm);
	mm.tc = backlog[i].tc;
	mm.to = backlog[i].to;
	mm.from = backlog[i].from;
	mm.source = backlog[i].source;
	mm.dest = backlog[i].dest;
	mm.pid = backlog[i].pid;
	mm.ptype = backlog[i].ptype;
	mm.cost = backlog[i].cost;
	mm.len = backlog[i].len;
	for (uint8_t j = 0; j < mm.len; j++)
		mm.data[j] = backlog[i].data[j];
	return mm;
}

uint16_t RfMeshMeshMessageBacklog::failed(uint16_t i) {
	if (backlog[i].redo == 0x80) {
		backlog[i].redo = 0;
		return backlog[i].to;
	}
	return RF_MESH_NOADDRESS;
}

void RfMeshMeshMessageBacklog::setTo(uint16_t i, uint8_t to) {
	backlog[i].to = to;
}

void RfMeshMeshMessageBacklog::done(uint8_t pid) {
	for (uint16_t i = 0; i < capacity; i++)
		if ((backlog[i].redo > 0) && (backlog[i].pid == pid)) {
			backlog[i].redo = 0;
#ifdef RF_MESH_NODETYPE_MASTER
//			LOG4CXX_INFO(logger, "RfMeshMeshMessageBacklog - done: " << RfMeshUtils::u16tos(i));		
#endif
		}
}

bool RfMeshMeshMessageBacklog::has(uint8_t pid) {
	for (uint16_t i = 0; i < capacity; i++)
		if ((backlog[i].redo > 0) && (backlog[i].pid == pid))
			return true;
	return false;
}
