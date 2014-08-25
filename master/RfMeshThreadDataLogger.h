#ifndef __RfMeshThreadDataLogger_h__
#define __RfMeshThreadDataLogger_h__

#include "RfMeshThreadNode.h"

class RfMeshThreadDataLogger : public RfMeshThreadBase {
	public:
		bool setup();
		bool loop();
	private:
		map<uint8_t,ofstream> mFiles;
		MessageQueue mqDataLogger;
		ofstream ofs;
};

#endif
