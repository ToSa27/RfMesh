#ifndef __RfMeshLeafBase_h__
#define __RfMeshLeafBase_h__

#include "RfMeshNodeConfig.h"

#include "RfMeshNode.h"
#include "RfMeshTransceiverRfm70.h"

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
