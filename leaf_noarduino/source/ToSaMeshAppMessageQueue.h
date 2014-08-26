#ifndef __RfMeshAppMessageQueue_h__
#define __RfMeshAppMessageQueue_h__

#include "RfMeshProtocol.h"

class RfMeshAppMessageQueue {
	private:
		uint16_t top;
		uint16_t len;
		int capacity;
		appMessage * queue;
    public:
		RfMeshAppMessageQueue(int capacity);
		~RfMeshAppMessageQueue();
		bool empty();
		void enqueue(appMessage am);
		appMessage dequeue();
};

#endif
