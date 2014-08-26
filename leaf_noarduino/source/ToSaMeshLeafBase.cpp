#include "RfMeshLeafBase.h"

uint16_t RfMeshLeafBase::calcCRC (const void* ptr, uint16_t len, uint16_t carryover) {
	uint16_t crc = carryover;
	for (uint16_t i = 0; i < len; ++i)
		crc = _crc16_update(crc, ((const char*) ptr)[i]);
	return crc;
}

uint16_t RfMeshLeafBase::calcCRC (const void* ptr, uint16_t len) {
	return calcCRC(ptr, len, (uint16_t)~0);
//	uint16_t crc = ~0;
//	for (uint16_t i = 0; i < len; ++i)
//		crc = _crc16_update(crc, ((const char*) ptr)[i]);
//	return crc;
}

void RfMeshLeafBase::reboot() {
	wdt_enable(WDTO_15MS);
	for (;;);
}

void RfMeshLeafBase::init() {
	node = new RfMeshNode(1, 10, 5, 5, 10);
	RfMeshTransceiverRfm70 *tc;
	tc = new RfMeshTransceiverRfm70(RFM70_SCLK, RFM70_MOSI, RFM70_MISO, RFM70_CSN, RFM70_CE);
	node->addTransceiver(tc);
	eeprom_read_block(&hardwareConfig, HARDWARE_CONFIG_ADDR, sizeof hardwareConfig);
	if (calcCRC(&hardwareConfig, sizeof hardwareConfig) != 0)
		reboot();
	eeprom_read_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
	if (calcCRC(&nodeConfig, sizeof nodeConfig) != 0)
		reboot();
	if (!node->Init(nodeConfig.address))
		reboot();
	node->SendHeartbeat(nodeConfig.rootAddress);
}
