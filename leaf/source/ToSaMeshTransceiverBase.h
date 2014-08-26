#ifndef __RfMeshTransceiverBase_h__
#define __RfMeshTransceiverBase_h__

#include "RfMeshNodeConfig.h"

class RfMeshTransceiverBase {
	protected:
		uint16_t addr;
		int index;
	public:
		void setIndex(int i);
		virtual bool Init(uint16_t addr) = 0;
        virtual void Send(meshMessage rm) = 0;
		virtual bool HasData() = 0;
		virtual meshMessage Receive() = 0;
};

#endif
