#ifndef __RfMeshMeshMessageBacklog_h__
#define __RfMeshMeshMessageBacklog_h__

#include "RfMeshNodeConfig.h"
#include "RfMeshProtocol.h"

#ifdef RF_MESH_NODETYPE_MASTER
#include "RfMeshLogger.h"
#include "RfMeshUtils.h"
#endif

typedef struct {
	uint8_t redo;
	unsigned long next;
    uint8_t tc;
    uint16_t to;
    uint16_t from;
    uint16_t source;
    uint16_t dest;
    uint8_t pid;
    uint8_t ptype;
    uint8_t cost;
    uint8_t len;
	uint8_t data[RF_MESH_MESH_MESSAGE_LEN_MAX];
} meshMessageBacklogEntry;

class RfMeshMeshMessageBacklog {
	private:
		meshMessageBacklogEntry * backlog;
#ifdef RF_MESH_NODETYPE_MASTER
		static log4cxx::LoggerPtr logger;
#endif
    public:
		RfMeshMeshMessageBacklog(int capacity);
		~RfMeshMeshMessageBacklog();
		int capacity;
		void add(meshMessage mm);
		void add(meshMessage mm, uint8_t redo);
		bool pending(uint16_t i);
		void reset(uint16_t i, bool submitted);
		void setTo(uint16_t i, uint8_t to);
		meshMessage get(uint16_t i);
		void done(uint8_t pid);
		bool has(uint8_t pid);
};

#endif
