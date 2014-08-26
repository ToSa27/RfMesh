#include "RfMeshRoutingTable.h"

RfMeshRoutingTable::RfMeshRoutingTable(int capacity) {
	this->capacity = capacity;
	routes = new routingTableEntry[capacity];
	for (uint16_t i = 0; i < capacity; i++)
		routes[i].dest = RF_MESH_NOADDRESS;
	pos = 0;
}

RfMeshRoutingTable::~RfMeshRoutingTable() {
	delete [] routes;
}

void RfMeshRoutingTable::is(uint16_t addr) {
	addr = addr;
}

void RfMeshRoutingTable::add(uint16_t dest, uint16_t hop, uint8_t tc, uint8_t cost) {
	if ((dest == addr) || (dest == RF_MESH_BROADCAST) || (dest == RF_MESH_NOADDRESS))
		return;
	for (uint16_t i = 0; i < capacity; i++) {
		if (routes[i].dest == dest) {
			if (routes[i].cost >= cost) {
				routes[i].hop = hop;
				routes[i].tc = tc;
				routes[i].cost = cost;
			}
			return;
		}
	}
	routes[pos].dest = dest;
	routes[pos].hop = hop;
	routes[pos].tc = tc;
	routes[pos].cost = cost;
	pos++;
	if (pos >= capacity)
		pos = 0;
}

bool RfMeshRoutingTable::has(uint16_t dest) {
	for (uint16_t i = 0; i < capacity; i++)
		if (routes[i].dest == dest)
			return true;
	return false;
}

uint16_t RfMeshRoutingTable::getHop(uint16_t dest) {
	for (uint16_t i = 0; i < capacity; i++)
		if (routes[i].dest == dest)
			return routes[i].hop;
	return RF_MESH_NOADDRESS;
}

uint8_t RfMeshRoutingTable::getTc(uint16_t dest) {
	for (uint16_t i = 0; i < capacity; i++)
		if (routes[i].dest == dest)
			return routes[i].tc;
	return 255;
}

uint8_t RfMeshRoutingTable::getCost(uint16_t dest) {
	for (uint16_t i = 0; i < capacity; i++)
		if (routes[i].dest == dest)
			return routes[i].cost;
	return 0;
}
