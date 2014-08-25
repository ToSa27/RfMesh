#ifndef __RfMeshThreadSocket_h__
#define __RfMeshThreadSocket_h__

#include "RfMeshThreading.h"

#define RF_MESH_SOCKET_BUF		1023
#define RF_MESH_SOCKET_FILE		"/tmp/RfMesh.sock"
#define RF_MESH_SOCKET_BACKLOG	5

class RfMeshThreadSocket : public RfMeshThreadBase {
	public:
		bool setup();
		bool loop();
	private:
		bool TickSocket();
		void TickToUi();
		void TickFromUi();
//		void TickRx();
//		int TickTx();
		int fdSocket;
		int fdClient;
};

#endif