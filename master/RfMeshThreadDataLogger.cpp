#include "RfMeshThreadDataLogger.h"

//#include <iostream>
//#include <fstream>
//using namespace std;

bool RfMeshThreadDataLogger::setup() {
	disp->mqSubscribe->enqueue(*this, new RfMeshThreadMessageSubscribe(&mqDataLogger, RF_MESH_PTYPE_DATASYNC, RF_MESH_NOADDRESS));
//	std::string fn = "DataLogger_F0.log";
//	ofs.open(fn.c_str(), std::ofstream::out);
//	ofs.open("DataLogger_F0.log");
	return true;
}

bool RfMeshThreadDataLogger::loop() {
	if (!mqDataLogger.empty()) {
		RfMeshThreadMessageAppPtr mp = (RfMeshThreadMessageAppPtr)mqDataLogger.dequeue(*this);
		appMessage am = mp->getMessage();
		delete mp;
//		if (mFiles.find(am.data[1]) == mFiles.end()) {
//			ofstream f;
//			std::string fn = "DataLogger_" + RfMeshUtils::u8tohex(am.data[1]) + ".log";
//			f.open(fn.c_str(), std::ofstream::out);
//			mFiles[am.data[1]] = f;
//		}
//		uint16_t val = am.data[2] + (am.data[3] << 8);
//		(ofstream)(mFiles[am.data[1]]) << RfMeshUtils::now() << " : " << RfMeshUtils::u16tohex(val) << std::endl << std::flush;		
//		ofs << RfMeshUtils::now() << " : " << RfMeshUtils::u16tohex(val) << std::endl << std::flush;		
		uint16_t raw = (am.data[3] << 8) + am.data[2];
		RfMeshDatabase::addDataLogging(am.data[1], raw);
	}
}
