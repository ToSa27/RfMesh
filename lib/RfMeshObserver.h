#ifndef __RfMeshObserver_h__
#define __RfMeshObserver_h__

#include "RfMeshNodeConfig.h"

class ILogMessageObserver {
	public:
		virtual void newLogMessage(std::string msgType, void *msgData) = 0;
};

class IMeshMessageObserver {
	public:
		std::vector<uint8_t> meshObservePTypes;
		std::vector<uint16_t> meshObserveNodes;
		virtual bool newMeshMessage(meshMessage mm) = 0;
		bool inMeshScope(meshMessage mm) {
			bool pvalid = false;
			if (meshObservePTypes.empty())
				pvalid = true;
			else
				for (int i = 0; i < meshObservePTypes.size(); i++)
					if (meshObservePTypes[i] == mm.ptype)
						pvalid = true;
			if (!pvalid)
				return false;
			bool nvalid = false;
			if (meshObserveNodes.empty())
				nvalid = true;
			else
				for (int i = 0; i < meshObserveNodes.size(); i++)
					if (meshObserveNodes[i] == mm.source)
						nvalid = true;
			if (!nvalid)
				return false;
			return true;
		}
};

class IAppMessageObserver {
	public:
		std::vector<uint8_t> appObservePTypes;
		std::vector<uint16_t> appObserveNodes;
		virtual bool newAppMessage(appMessage am) = 0;
		bool inAppScope(appMessage am) {
			bool pvalid = false;
			if (appObservePTypes.empty())
				pvalid = true;
			else
				for (int i = 0; i < appObservePTypes.size(); i++)
					if (appObservePTypes[i] == am.ptype)
						pvalid = true;
			if (!pvalid)
				return false;
			bool nvalid = false;
			if (appObserveNodes.empty())
				nvalid = true;
			else
				for (int i = 0; i < appObserveNodes.size(); i++)
					if (appObserveNodes[i] == am.source)
						nvalid = true;
			if (!nvalid)
				return false;
			return true;
		}
};

#endif