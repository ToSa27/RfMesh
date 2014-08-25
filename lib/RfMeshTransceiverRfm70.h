#ifndef __RfMeshTransceiverRfm70_h__
#define __RfMeshTransceiverRfm70_h__

#include "RfMeshNodeConfig.h"

#include "RfMeshTransceiverBase.h"
#include "rfm70.h"

typedef struct {
    uint16_t to;
    uint8_t len;
	uint8_t data[RF_MESH_RAW_MESSAGE_LEN_MAX];
} rawMessageRfm70;

class RfMeshTransceiverRfm70 : public RfMeshTransceiverBase, private rfm70 {
	private:
		void SetAddress(uint16_t addr, bool brdcst);
		meshMessage RawToMesh(rawMessageRfm70 rm);
		rawMessageRfm70 MeshToRaw(meshMessage mm);
    public:
		RfMeshTransceiverRfm70(uint8_t sclk, uint8_t mosi, uint8_t miso, uint8_t csn, uint8_t ce);
		virtual bool Init(uint16_t addr);
        virtual void Send(meshMessage mm);
		virtual bool HasData();
        virtual meshMessage Receive();
};

#endif
