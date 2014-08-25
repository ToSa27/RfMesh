#ifndef __RfMeshDatabase_h__
#define __RfMeshDatabase_h__

#include "RfMeshNodeConfig.h"

#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
 
#include <mongo/client/dbclient.h>

#define RF_MESH_DB_NAME	RfMeshDb

using namespace std;
using namespace xercesc;
using namespace mongo;

struct Firmware {
	uint16_t hwType;
	uint16_t hwVersion;
	uint16_t fwVersion;
	uint16_t fwBlockCount;
	uint16_t fwCrc;
};

class RfMeshDatabase {
//	private:
	public:
		static log4cxx::LoggerPtr logger;
		static DBClientConnection c;
//		void importData(std::string filename);
		static bool execute(char* query);
		static std::vector<std::vector<std::string> > query(char* query);
		static uint16_t crc16_update(uint16_t crc, uint8_t a);
		static void addFloor(std::string floor);
		static void addRoom(std::string floor, std::string room);
		static int getFloorId(std::string floor);
		static int getRoomId(std::string floor, std::string room);
		static int getHardwareId(uint16_t hwType, uint16_t hwVersion);
		static int getFirmwareId(uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion);
		static int getNextId(std::string table);
		static int getNextId(std::string table, std::string column);
		static std::string getSetting(std::string key);
		static void checkFirmwareDirectory();
		static void loadFirmwareFile(std::string fwHexFile);
		static BSONObj getFirmware(uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion);
		static std::string macBtoS(uint8_t *in);
		static uint8_t * macStoB(std::string in);
		static uint16_t getNextAvailableAddress();
//	public:
//		RfMeshDatabase();
//		~RfMeshDatabase();	
		static void init();
		static uint8_t * getNewMac();
		static uint16_t getAddress(uint8_t * mac);
		static bool nodeExists(uint16_t address);
		static bool addNode(uint16_t address, std::string macAddress, std::string floor, std::string room, std::string name);
//		void editNodeDetails(uint16_t address, std::string hwName, uint16_t hwVersion, uint16_t fwVersion);
		static void editNodeDetails(uint16_t address, uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion, uint16_t fwCrc);
		static vector<uint16_t> getNodes();
		static bool nodeAutoUpdate(uint16_t address);
		static bool needsUpdate(uint16_t address);
		static Firmware getLatestFirmware(uint16_t address);
		static Firmware getLatestFirmware(uint16_t hwType, uint16_t hwVersion);
		static void getFirmwareBlock(uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion, uint16_t fwBlockIndex, uint8_t data[]);
		static uint8_t getNodeConfig(uint16_t address, uint8_t configType, uint8_t configId, uint8_t data[]);
		static uint16_t getConfigCrc(uint16_t address);
		static uint16_t getConfigCrcHelper(BSONObj boc, uint16_t crc);
		static void addHardware(std::string hwName, uint16_t hwType, uint16_t hwVersion);
//		int getHardwareType(std::string hwName);
		static void addFirmware(uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion, std::string hexFile);
		static void addDataLogging(uint8_t address, uint16_t raw);
		static map<uint8_t,uint8_t> memPrototype;
		static vector<string> getDataLogging(uint8_t address);
};

#endif
