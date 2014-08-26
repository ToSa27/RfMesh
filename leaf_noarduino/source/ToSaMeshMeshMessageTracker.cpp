#include "RfMeshMeshMessageTracker.h"

RfMeshMeshMessageTracker::RfMeshMeshMessageTracker(int capacity) {
	this->capacity = capacity;
	tracker = new meshMessageTrackerEntry[capacity];
	for (uint16_t i = 0; i < capacity; i++)
		tracker[i].addr = RF_MESH_NOADDRESS;
}

RfMeshMeshMessageTracker::~RfMeshMeshMessageTracker() {
	delete [] tracker;
}

uint8_t RfMeshMeshMessageTracker::get(uint16_t addr) {
	for (uint16_t i = 0; i < capacity; i++)
		if (tracker[i].addr == addr)
			return tracker[i].pid;
	return 0xFF;
}

void RfMeshMeshMessageTracker::set(uint16_t addr, uint8_t pid) {
	for (uint16_t i = 0; i < capacity; i++)
		if (tracker[i].addr == addr) {
			tracker[i].pid = pid;
			return;
		}
	for (uint16_t i = 0; i < capacity; i++)
		if (tracker[i].addr == RF_MESH_NOADDRESS) {
			tracker[i].addr = addr;
			tracker[i].pid = pid;
			return;
		}
	// ToDo : handle tracker overflow
}
