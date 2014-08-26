#ifndef __RfMeshRoutingTable_h__
#define __RfMeshRoutingTable_h__

#include "RfMeshProtocol.h"

typedef struct {
	uint16_t dest;
	uint16_t hop;
	uint8_t tc;
	uint8_t cost;
} routingTableEntry;

class RfMeshRoutingTable {
	private:
		uint16_t addr;
		uint16_t pos;
		int capacity;
		routingTableEntry * routes;
    public:
		RfMeshRoutingTable(int capacity);
		~RfMeshRoutingTable();
		void is(uint16_t addr);
		void add(uint16_t dest, uint16_t hop, uint8_t tc, uint8_t cost);
		bool has(uint16_t dest);
		uint16_t getHop(uint16_t dest);
		uint8_t getTc(uint16_t dest);
		uint8_t getCost(uint16_t dest);
};

#endif
