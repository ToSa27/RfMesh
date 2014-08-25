#ifndef __RfMeshNode_h__
#define __RfMeshNode_h__

#include "RfMeshNodeConfig.h"

#include "RfMeshTransceiverBase.h"
#include "RfMeshRoutingTable.h"
#include "RfMeshAppMessageQueue.h"
#include "RfMeshMeshMessageBacklog.h"
#include "RfMeshMeshMessageTracker.h"

class RfMeshNode {
	private:
#ifdef RF_MESH_NODETYPE_MASTER
		static log4cxx::LoggerPtr logger;
//		bool timeTrigger;
//		unsigned long timeTriggerMillis;
#endif
		RfMeshRoutingTable *rt;
		RfMeshAppMessageQueue *rxq;
		RfMeshMeshMessageBacklog *txb;
		RfMeshMeshMessageTracker *rxt;
		appMessage MeshToApp(meshMessage rm);
		meshMessage AppToMesh(appMessage mm);
		void LostRoute(int rti);
		void SendArpRequest(uint16_t dest);
		void SendDhcpRequest();
		void SendPing(uint16_t dest);
		bool Forward(meshMessage mm, bool isBroadcast, bool addAddressToData);
#ifdef RF_MESH_NODETYPE_MASTER
		std::vector<ILogMessageObserver*> logObservers;
		std::vector<IMeshMessageObserver*> meshObservers;
		std::vector<IAppMessageObserver*> appObservers;
		void notifyLogObserver(std::string msgType, void *msgData);
		bool notifyMeshObserver(meshMessage mm);
		bool notifyAppObserver(appMessage am);
//		std::vector<uint8_t> answerBroadcastTypes;
#endif
		RfMeshTransceiverBase** tcs;
		uint8_t tcsc;
	public:
		RfMeshNode(int transceiverCount, int routingTableCapacity, int AppMessageQueueCapacity, int MeshMessageBacklogCapacity, int MeshMessageTrackerCapacity);
		~RfMeshNode();
		bool Init(uint16_t newaddr);
		bool Send(appMessage Message);
		void SendWaitHopAck(meshMessage mm);
		void SendHeartbeat(uint16_t dest);
		bool HasData();
		appMessage Receive();
		void Tick();
		void TickTx();
		void TickRx();
		void addTransceiver(RfMeshTransceiverBase *tcvr);
#ifdef RF_MESH_NODETYPE_MASTER
		static appMessage getTimeMessage();
		void addLogObserver(ILogMessageObserver* obj);
		void addMeshObserver(IMeshMessageObserver* obj);
		void addAppObserver(IAppMessageObserver* obj);
//		void answerBroadcastType(uint8_t ptype);
#endif
		uint16_t addr;
		bool SetAddress(uint16_t newAddr);
		uint8_t macAddress[6];
		void SetMacAddress(uint8_t *newMacAddr);
		uint8_t nextpid;
		void Send(meshMessage Message);
#ifdef RF_MESH_NODETYPE_LEAF
		bool tsValid;
		Timestamp tsLast;
		uint32_t tsMillis;
		void refreshTime();
		Datestamp nextBankHoliday;
		Datestamp nextVacationStart;
		Datestamp nextVacationEnd;
#endif
};

#endif
