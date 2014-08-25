#include "RfMeshDatabase.h"

DBClientConnection RfMeshDatabase::c;
map<uint8_t,uint8_t> RfMeshDatabase::memPrototype;

LoggerPtr RfMeshDatabase::logger(Logger::getLogger("ToSa.Mesh"));

void RfMeshDatabase::init() {
	c.connect("localhost:8017");
	if (c.count("RfMeshDb.Settings") < 1) {
		BSONObj s = BSON( "key" << "NextAvailableAddress" << "value" << 2 );
		c.insert("RfMeshDb.Settings", s);
		s = BSON( "key" << "FirmwareDirectory" << "value" << "../firmware/" );
		c.insert("RfMeshDb.Settings", s);
	}
	checkFirmwareDirectory();
}

std::string RfMeshDatabase::getSetting(std::string key) {
	std::auto_ptr<DBClientCursor> cursor = c.query("RfMeshDb.Settings", QUERY("key" << key));
	if (!cursor.get())
		return "";
	if (cursor->more())	{
		BSONObj p = cursor->next();
		return p.getStringField("value");
    }
	return "";
}

void RfMeshDatabase::checkFirmwareDirectory() {
	std::string dirname = getSetting("FirmwareDirectory");
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(dirname.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			std::string entname = ent->d_name;
			if (entname.size() > 4)
				if (entname.rfind(".hex", std::string::npos) == entname.size() - 4) {
					std::string fullfilename = dirname + entname;
					loadFirmwareFile(fullfilename);
				}
		}
		closedir (dir);
	}
}

void RfMeshDatabase::loadFirmwareFile(std::string fwHexFile) {
	std::string fwHexFilename = fwHexFile;
	if (fwHexFilename.find_last_of('/', std::string::npos) != std::string::npos)
		fwHexFilename = fwHexFilename.substr(fwHexFilename.find_last_of('/', std::string::npos) + 1, std::string::npos);
	vector<std::string> subs = RfMeshUtils::split(fwHexFilename, '_');
	if (subs.size() < 3)
		return;
	int hwType = atoi(subs[0].c_str());
	int hwVersion = atoi(subs[1].c_str());
	int fwVersion = atoi(subs[2].c_str());
	addFirmware(hwType, hwVersion, fwVersion, fwHexFile);
}

uint8_t * RfMeshDatabase::getNewMac() {
	uint8_t mac[6];
	mac[0] =  84; // 0x54 - T
	mac[1] = 111; // 0x6F - o
	mac[2] =  83; // 0x53 - S
	mac[3] =  97; // 0x61 - a
	mac[4] =   0; // 0x00
	mac[5] =   1; // 0x01
	std::string macs;
	for(;;) {
		macs = macBtoS(mac);
		if (c.count("RfMeshDb.Node", BSON("mac" << macs)) == 0)
			break;
		if (mac[5] < 255)
			mac[5] += 1;
		else {
			mac[4] += 1;
			mac[5] = 0;
		}
	}
	BSONObj s = BSON( "mac" << macs );
	c.insert("RfMeshDb.Node", s);
	return macStoB(macs);
}

std::string RfMeshDatabase::macBtoS(uint8_t *in) {
	std::string res = "";
	for (uint8_t i = 0; i < 6; i++) {
		if (i > 0)
			res += ":";
		res += RfMeshUtils::u8tohex(in[i]);
	}
	return res;
}

uint8_t * RfMeshDatabase::macStoB(std::string in) {
	uint8_t res[6];
	for (uint8_t i = 0; i < 6; i++)
		res[i] = RfMeshUtils::hextou8(in.substr(3 * i, 2));
	return res;
}

uint16_t RfMeshDatabase::getAddress(uint8_t * mac) {
	std::string macs = macBtoS(mac);
	uint16_t addr = RF_MESH_NOADDRESS;
	std::auto_ptr<DBClientCursor> cursor = c.query("RfMeshDb.Node", QUERY("mac" << macs));
	bool macExists = false;
	if (cursor.get()) {
		if (cursor->more())	{
			macExists = true;
			BSONObj p = cursor->next();
			if (p.hasField("address"))
				addr = (uint16_t)p.getIntField("address");
		}
	}
	if (addr == RF_MESH_NOADDRESS) {
		addr = getNextAvailableAddress();
		if (macExists) {
			c.update("RfMeshDb.Node", QUERY("mac" << macs), BSON("$set" << BSON("address" << addr)));
		} else {
			BSONObj s = BSON( "mac" << macs << "address" << addr);
			c.insert("RfMeshDb.Node", s);
		}
	}
	return addr;
}

uint16_t RfMeshDatabase::getNextAvailableAddress() {
	for (uint16_t addr = 0x0002; addr < 0xFFFF; addr++)
		if (!nodeExists(addr))
			return addr;
	return RF_MESH_NOADDRESS;
}

bool RfMeshDatabase::nodeExists(uint16_t address) {
	return (c.count("RfMeshDb.Node", BSON("address" << address)) > 0);
}

void RfMeshDatabase::editNodeDetails(uint16_t address, uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion, uint16_t fwCrc) {
	c.update("RfMeshDb.Node", QUERY("address" << address), BSON("$set" << BSON("hwType" << hwType << "hwVersion" << hwVersion << "fwVersion" << fwVersion << "fwCrc" << fwCrc)));
}

vector<uint16_t> RfMeshDatabase::getNodes() {
	vector<uint16_t> res;
	std::auto_ptr<DBClientCursor> cursor;
	cursor = c.query("RfMeshDb.Node");
	if (cursor.get())
		while (cursor->more()) {
			BSONObj b = cursor->next();
			res.push_back((uint16_t)b.getIntField("address"));
		}
	return res;
}

bool RfMeshDatabase::nodeAutoUpdate(uint16_t address) {
	std::auto_ptr<DBClientCursor> cursor;
	cursor = c.query("RfMeshDb.Node", QUERY("address" << address));
	if (cursor.get())
		if (cursor->more()) {
			BSONObj b = cursor->next();
			if (b.hasField("autoUpdate"))
				return (bool)b.getBoolField("autoUpdate");
			else
				return true;
		}
	return true;
}

bool RfMeshDatabase::needsUpdate(uint16_t address) {
	std::auto_ptr<DBClientCursor> cursor;
	cursor = c.query("RfMeshDb.Node", QUERY("address" << address));
	if (cursor.get())
		if (cursor->more()) {
			BSONObj b = cursor->next();
			if (b.hasField("hwType") && b.hasField("hwVersion") && b.hasField("fwVersion") && b.hasField("fwCrc")) {
				uint16_t hwType = (uint16_t)b.getIntField("hwType");
				uint16_t hwVersion = (uint16_t)b.getIntField("hwVersion");
				uint16_t fwVersion = (uint16_t)b.getIntField("fwVersion");
				uint16_t fwCrc = (uint16_t)b.getIntField("fwCrc");
				Firmware fw = getLatestFirmware(hwType, hwVersion);
				if ((fw.fwVersion == fwVersion) && (fw.fwCrc == fwCrc))
					return false;
			} else
				return true;
		}
	return true;
}

Firmware RfMeshDatabase::getLatestFirmware(uint16_t address) {
	Firmware fw;
	std::auto_ptr<DBClientCursor> cursor;
	cursor = c.query("RfMeshDb.Node", QUERY("address" << address));
	if (cursor.get())
		if (cursor->more()) {
			BSONObj b = cursor->next();
			uint16_t hwType = (uint16_t)b.getIntField("hwType");
			uint16_t hwVersion = (uint16_t)b.getIntField("hwVersion");
			fw = getLatestFirmware(hwType, hwVersion);
		}
	return fw;
}

Firmware RfMeshDatabase::getLatestFirmware(uint16_t hwType, uint16_t hwVersion) {
	BSONObj boFw = getFirmware(hwType, hwVersion, 0);	
	Firmware fw;
	fw.hwType = (uint16_t)boFw.getIntField("hwType");
	fw.hwVersion = (uint16_t)boFw.getIntField("hwVersion");
	fw.fwVersion = (uint16_t)boFw.getIntField("fwVersion");
	fw.fwBlockCount = 0;
	BSONForEach(block, boFw.getObjectField("blocks"))
		(fw.fwBlockCount)++;
	fw.fwCrc = (uint16_t)boFw.getIntField("crc");
	return fw;
}

void RfMeshDatabase::getFirmwareBlock(uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion, uint16_t fwBlockIndex, uint8_t data[]) {
	BSONObj boFw = getFirmware(hwType, hwVersion, fwVersion);
	BSONObj boBlocks = boFw.getObjectField("blocks");
	BSONElement beBlock = boBlocks[(int)fwBlockIndex];
	BSONObj boBlock = beBlock.Obj();
	for (int i = 0; i < FIRMWARE_BLOCK_SIZE; i++)
		data[i] = (uint8_t)(boBlock[i].Int());
}

uint8_t RfMeshDatabase::getNodeConfig(uint16_t address, uint8_t configType, uint8_t configId, uint8_t data[]) {
	data[0] = configType;
	data[1] = configId;
	uint8_t maskedConfigType = configType & 0x7F; //!RF_MESH_CONFIG_FETCHALL;
	std::auto_ptr<DBClientCursor> cursor;
	cursor = c.query("RfMeshDb.Node", QUERY("address" << address));
	if (cursor.get()) {
		if (cursor->more()) {
			BSONObj boNode = cursor->next();
			BSONObj boConfig = boNode.getObjectField("config");
			if (maskedConfigType == RF_MESH_CONFIG_TYPE_HEADER) {
				data[2] = (uint8_t)boConfig.getObjectField("io").nFields();
				data[3] = (uint8_t)boConfig.getObjectField("network").nFields();
				data[4] = (uint8_t)boConfig.getObjectField("control").nFields();
				uint16_t paramCount = (uint16_t)boConfig.getObjectField("parameter").nFields();
				data[5] = (uint8_t)(paramCount & 0x00FF);
				data[6] = (uint8_t)((paramCount & 0xFF00) >> 8);
				data[7] = (uint8_t)boConfig.getObjectField("timer").nFields();
				return 8;
			}
			BSONObj boc;
			switch (maskedConfigType) {
				case RF_MESH_CONFIG_TYPE_IO:
					boc = boConfig.getObjectField("io");
					break;
				case RF_MESH_CONFIG_TYPE_NETWORK:
					boc = boConfig.getObjectField("network");
					break;
				case RF_MESH_CONFIG_TYPE_CONTROL:
					boc = boConfig.getObjectField("control");
					break;
				case RF_MESH_CONFIG_TYPE_PARAMETER:
					boc = boConfig.getObjectField("parameter");
					break;
				case RF_MESH_CONFIG_TYPE_TIMER:
					boc = boConfig.getObjectField("timer");
					break;
			}
			BSONElement be = boc[(int)configId];
			BSONObj bo = be.Obj();
			for (int i = 0; i < bo.nFields(); i++)
				data[2 + i] = (uint8_t)(bo[i].Int());
			return (2 + bo.nFields());
		}
	}
	return 0;
}

uint16_t RfMeshDatabase::getConfigCrcHelper(BSONObj boc, uint16_t crc) {
	uint16_t res = crc;
	for (int i = 0; i < boc.nFields(); i++) {
		BSONElement be = boc[i];
		BSONObj bo = be.Obj();
		for (int j = 0; i < bo.nFields(); i++)
			res = crc16_update(res, (uint8_t)(bo[j].Int()));
	}
	return res;
}

uint16_t RfMeshDatabase::getConfigCrc(uint16_t address) {
	std::auto_ptr<DBClientCursor> cursor;
	cursor = c.query("RfMeshDb.Node", QUERY("address" << address));
	if (cursor.get()) {
		if (cursor->more()) {
			BSONObj boNode = cursor->next();
			BSONObj boConfig = boNode.getObjectField("config");
			uint16_t crc = 0xFFFF;
			crc = getConfigCrcHelper(boConfig.getObjectField("io"), crc);
			crc = getConfigCrcHelper(boConfig.getObjectField("network"), crc);
			crc = getConfigCrcHelper(boConfig.getObjectField("control"), crc);
			crc = getConfigCrcHelper(boConfig.getObjectField("parameter"), crc);
			crc = getConfigCrcHelper(boConfig.getObjectField("timer"), crc);
			return crc;
		}
	}
	return 0;
}

// gets specific firmware version
// if fwVersion parameter is null then get the latest available firmware version
BSONObj RfMeshDatabase::getFirmware(uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion) {
	std::auto_ptr<DBClientCursor> cursor;
	if (fwVersion == 0)
		cursor = c.query("RfMeshDb.Firmware", QUERY("hwType" << hwType << "hwVersion" << hwVersion).sort("fwVersion", -1));
	else
		cursor = c.query("RfMeshDb.Firmware", QUERY("hwType" << hwType << "hwVersion" << hwVersion << "fwVersion" << fwVersion));
	if (cursor.get())
		if (cursor->more())
			return cursor->next().copy();
	return BSONObj();
}

void RfMeshDatabase::addFirmware(uint16_t hwType, uint16_t hwVersion, uint16_t fwVersion, std::string hexFile) {
	LOG4CXX_INFO(logger, "Firmware check: " << hwType << "/" << hwVersion << "/" << fwVersion << " -> " << hexFile);
	BSONArrayBuilder babBlocks;
	uint16_t blocks = 0;
	uint16_t crc = 0;
	char sql[1000];
	if (!hexFile.empty()) {
		std::ifstream hf(hexFile.c_str());
		if (!hf)
			LOG4CXX_ERROR(logger, "file not found: " << hexFile);			
		std::vector<uint8_t> bytes;
		uint16_t start = 0;
		uint16_t end = 0;
		for (std::string line; getline(hf, line);) {
			uint8_t reclen = RfMeshUtils::hextou8(line.substr(1, 2));
			uint16_t offset = RfMeshUtils::hextou16(line.substr(3, 4));
			uint8_t rectype = RfMeshUtils::hextou8(line.substr(7, 2));
			std::string data = line.substr(9, 2 * reclen);
			uint8_t chksum = RfMeshUtils::hextou8(line.substr(9 + (2 * reclen), 2));
			if (rectype == 0) {
				if ((start == 0) && (end == 0)) {
					if (offset % 128 > 0)
						return;
					start = offset;
					end = offset;
				}
				if (offset < end)
					return;
				while (offset > end) {
					bytes.push_back(255);
					end++;
				}
				for (uint8_t i = 0; i < reclen; i++)
					bytes.push_back(RfMeshUtils::hextou8(data.substr(i * 2, 2)));
				end += reclen;
			}
		}
		uint16_t pad = end % 128; // ATMega328 has 64 words per page / 128 bytes per page
		for (uint16_t i = 0; i < 128 - pad; i++) {
			bytes.push_back(255);
			end++;
		}
		blocks = (end - start) / FIRMWARE_BLOCK_SIZE;
		for (uint16_t block = 0; block < blocks; block++) {
			BSONArrayBuilder babBlock;
			for (uint16_t i = 0; i < FIRMWARE_BLOCK_SIZE; i++)
				babBlock.append(bytes[(block * FIRMWARE_BLOCK_SIZE) + i]);
			babBlocks.append(babBlock.arr());
		}
		crc = ~0;
		for (uint16_t i = 0; i < blocks * FIRMWARE_BLOCK_SIZE; ++i)
			crc = crc16_update(crc, bytes[i]);
		bool newFw = false;
		BSONObj boOldFw = getFirmware(hwType, hwVersion, fwVersion);
		if (boOldFw.isEmpty())
			newFw = true;
		else if (crc != (uint16_t)boOldFw.getIntField("crc"))
			newFw = true;
		if (newFw) {
			c.remove("RfMeshDb.Firmware", BSON("hwType" << hwType << "hwVersion" << hwVersion << "fwVersion" << fwVersion));
			BSONObj boFw = BSON("hwType" << hwType << "hwVersion" << hwVersion << "fwVersion" << fwVersion << "crc" << crc << "blocks" << babBlocks.arr() << "hexFile" << hexFile);
			c.insert("RfMeshDb.Firmware", boFw);
			LOG4CXX_INFO(logger, "Firmware updated: " << hwType << "/" << hwVersion << "/" << fwVersion << " -> " << hexFile);
		} else {
			LOG4CXX_INFO(logger, "Firmware unchanged: " << hwType << "/" << hwVersion << "/" << fwVersion << " -> " << hexFile);			
		}
	}
}

uint16_t RfMeshDatabase::crc16_update(uint16_t crc, uint8_t a) {
	int i;
	crc ^= a;
	for (i = 0; i < 8; ++i) {
		if (crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = (crc >> 1);
	}
	return crc;
}

void RfMeshDatabase::addDataLogging(uint8_t address, uint16_t raw) {
	if (memPrototype[address] == NULL) {
		// get document from DataLogger collection with key = config
		// get mem array
		// get entry with address
		// get prototype from entry
		std::auto_ptr<DBClientCursor> cursor;
		cursor = c.query("RfMeshDb.Configuration", QUERY("key" << "DataLogger"));
		if (cursor.get()) {
			if (cursor->more()) {
				BSONObj boDataLogger = cursor->next();
				BSONObj boMems = (BSONArray)boDataLogger.getObjectField("mem");
				for (int i = 0; i < boMems.nFields(); i++) {
					BSONElement beMem = boMems[i];
					BSONObj boMem = beMem.Obj();
					if ((uint8_t)boMem.getIntField("address") == address) {
						memPrototype[address] = (uint8_t)boMem.getIntField("prototype");
						break;
					}
				}
			}
		}
	}
	if (memPrototype[address] == NULL)
		return;
	uint8_t t = memPrototype[address];
	BSONObj bo;
	switch (t) {
		case 1: // DS1820 temperature
			double d;
			if (raw & 0x8000) {
				uint16_t rawp = (raw ^ 0xFFFF) + 1;
				d = -(rawp / 32.0);
			} else {
				d = (raw / 32.0);
			}
//			bo = BSON("address" << address << "rawdata" << BSON_ARRAY(rawdataHigh << rawdataLow) << "data" << d);
			bo = BSON("address" << address << "raw" << raw << "data" << d);
			break;
		case 2: // OnOff
			string s;
			if (raw == 0x0000)
				s = "off";
			else if (raw == 0x0001)
				s = "on";
			else s = "unknown";
//			bo = BSON("address" << address << "rawdata" << BSON_ARRAY(rawdataHigh << rawdataLow) << "data" << s);
			bo = BSON("address" << address << "raw" << raw << "data" << s);
			break;
	}
	c.insert("RfMeshDb.DataLogger", bo);
}

vector<string> RfMeshDatabase::getDataLogging(uint8_t address) {
	vector<string> res;
	std::auto_ptr<DBClientCursor> cursor = c.query("RfMeshDb.DataLogger", QUERY("address" << address));
	if (!cursor.get())
		return res;
	while (cursor->more())	{
		BSONObj p = cursor->next();
//		BSONElement pid;
//		p.getObjectID(&p);
//		BSONElement pid = p.getField("_id");
//		BSONElement pval = p.getField("value");
//		Time t = new Time(parseInt(pid.toString().substring(0,8), 16) * 1000);
//		res.add(t, pval.Double());
		res.push_back(p.jsonString());
    }
	return res;
}
