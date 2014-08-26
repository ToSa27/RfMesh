#ifndef __RfMeshLeafBase_h__
#define __RfMeshLeafBase_h__

#include "RfMeshNodeConfig.h"
#include "RfMeshNode.h"
#include "RfMeshTransceiverRfm70.h"
#include "RfMeshProtocol.h"

#define	RFM70_SCLK	13
#define	RFM70_MOSI	11
#define	RFM70_MISO	12
#define	RFM70_CSN	10
#define	RFM70_CE	8

/*
typedef struct HardwareConfig {
	uint16_t hwType;
	uint16_t hwVersion;
	uint8_t macAddress[6];
	uint16_t crc;
};

typedef struct NodeConfig {
	uint16_t address;
	uint16_t rootAddress;
	uint16_t fwVersion;
	uint16_t fwBlockCount;
	uint16_t fwCrc;
	uint16_t configCrc;
	uint16_t crc;
};
*/

class RfMeshLeafBase {
	protected:
		RfMeshNode *node;
		HardwareConfig hardwareConfig;
		NodeConfig nodeConfig;
		uint16_t calcCRC (const void* ptr, uint16_t len, uint16_t carryover);
		uint16_t calcCRC (const void* ptr, uint16_t len);
		void reboot();
	public:
		void init();
};

#endif
