#ifndef __RfMeshThreadNode_h__
#define __RfMeshThreadNode_h__

#include "RfMeshThreading.h"

#include "RfMeshObserver.h"
#include "RfMeshNode.h"
#include "RfMeshDatabase.h"
#include "RfMeshTransceiverRfm70.h"

class RfMeshAppSubscriber : public IAppMessageObserver {
	private:
		MessageQueue* queue;
		Thread* thread;
	public:
		RfMeshAppSubscriber(uint8_t ptype, uint16_t node, MessageQueue* queue, Thread* thread) {
			if (ptype != RF_MESH_PTYPE_UNKNOWN)
				this->appObservePTypes.push_back(ptype);
			if (node != RF_MESH_NOADDRESS)
				this->appObserveNodes.push_back(node);
			this->queue = queue;
			this->thread = thread;
		}
		bool newAppMessage(appMessage am) {
			queue->enqueue(*thread, new RfMeshThreadMessageApp(am));
		}
};

class RfMeshThreadNode : public RfMeshThreadBase, public ILogMessageObserver, public IMeshMessageObserver, public IAppMessageObserver {
//class RfMeshThreadNode : public RfMeshThreadBase {
	public:
		bool setup();
		bool loop();
		virtual void newLogMessage(std::string msgType, void *msgData);
		virtual bool newMeshMessage(meshMessage mm);
		virtual bool newAppMessage(appMessage am);
	private:
//		RfMeshDatabase *db;
		RfMeshTransceiverBase *tc;
		RfMeshNode *node;
		vector<RfMeshAppSubscriber*> subscribers;
		void handleUiRequest(std::string json);
		void SendWhoAmIResponse(meshMessage mm);
		void SendDhcpResponse(meshMessage mm);
		void SendAnnounceResponse(uint16_t address);
		void SendAnnounceResponse(meshMessage mm);
		void SendFirmwareResponse(meshMessage mm);
		void SendConfigResponse(meshMessage mm);
};

#endif