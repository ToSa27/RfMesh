#ifndef __RfMeshMeshMessageTracker_h__
#define __RfMeshMeshMessageTracker_h__

#include "RfMeshNodeConfig.h"

typedef struct {
	uint16_t addr;
	uint8_t pid;
} meshMessageTrackerEntry;

class RfMeshMeshMessageTracker {
	private:
		int capacity;
		meshMessageTrackerEntry * tracker;
    public:
		RfMeshMeshMessageTracker(int capacity);
		~RfMeshMeshMessageTracker();
		uint8_t get(uint16_t addr);
		void set(uint16_t addr, uint8_t pid);
};

#endif
