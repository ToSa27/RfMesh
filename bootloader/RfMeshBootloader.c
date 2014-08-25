#include "RfMeshNodeConfig.h"

#include "RfMeshBootloaderRfm70.h"

struct HardwareConfig hardwareConfig;
struct NodeConfig nodeConfig;

uint8_t progBuf[SPM_PAGESIZE];
uint8_t nextpid;

meshMessage mmRequest;
meshMessage mmResponse;
meshMessage mmHopAck;

int main(void) __attribute__ ((naked)) __attribute__ ((section (".init9")));

volatile char dummy;

EMPTY_INTERRUPT(WDT_vect);

static uint16_t calcCRC (const void* ptr, uint16_t len) {
  uint16_t crc = ~0;
  for (uint16_t i = 0; i < len; ++i)
    crc = _crc16_update(crc, ((const char*) ptr)[i]);
  return crc;
}

static uint16_t calcCRCrom (const void* ptr, uint16_t len) {
	uint16_t crc = ~0;
	for (uint16_t i = 0; i < len; ++i)
		crc = _crc16_update(crc, pgm_read_byte((uint16_t) ptr + i));
	return crc;
}

static uint8_t validFirmware () {
	return calcCRCrom(0, nodeConfig.fwBlockCount * FIRMWARE_BLOCK_SIZE) == nodeConfig.fwCrc;
}

static void reboot() {
  wdt_enable(WDTO_15MS);
  for (;;);
}

static void startup() {
	if (validFirmware()) {
		led(LED_BITMASK_GREEN, 200);
		clock_prescale_set(clock_div_1);
		((void(*)()) 0)();
	} else
		reboot();
}

static void boot_program_page (uint32_t page, uint8_t *buf) {
  uint8_t sreg = SREG;
  cli();
  eeprom_busy_wait();
  boot_page_erase(page);
  boot_spm_busy_wait();
  for (uint16_t i = 0; i < SPM_PAGESIZE; i += 2) {
    uint16_t w = *buf++;
    w += (*buf++) << 8;
    boot_page_fill(page + i, w);
  }
  boot_page_write(page);
  boot_spm_busy_wait();
  boot_rww_enable();
  SREG = sreg;
}

static void SendHopAck() {
	mmHopAck.source = mmResponse.dest;
	mmHopAck.from = TransceiverAddress;
	mmHopAck.dest = mmResponse.source;
	mmHopAck.to = mmResponse.from;
	mmHopAck.pid = nextpid++;
	mmHopAck.ptype = RF_MESH_PTYPE_HOPACK;
	mmHopAck.cost = 1;
	mmHopAck.len = 1;
	mmHopAck.data[0] = mmResponse.pid;
	TransceiverSend(mmHopAck);
}

static uint8_t SendAndWait(uint8_t reqType, uint8_t resType) {
	mmRequest.pid = nextpid++;
	mmRequest.ptype = reqType;
	mmRequest.cost = 1;
	for (uint8_t i = 0; i < 10; i++) {
		TransceiverSend(mmRequest);
		for (uint16_t j = 0; j < 20; j++) {
			wdt_reset();
			for (uint16_t j = 0; j < 1000; j++) {
				if (TransceiverHasData()) {
					mmResponse = TransceiverReceive();
					if ((mmResponse.ptype & 0x80) > 0)
						SendHopAck();
					if (mmResponse.ptype == resType) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

static void run () {
	wdt_reset();
	nextpid = 0;
	TransceiverInit();
	mmRequest.pid = 0;
	wdt_reset();
	eeprom_read_block(&hardwareConfig, HARDWARE_CONFIG_ADDR, sizeof hardwareConfig);
	if (calcCRC(&hardwareConfig, sizeof hardwareConfig) != 0) {
		led(LED_BITMASK_BOTH, 0);
		mmRequest.source = RF_MESH_NOADDRESS;
		mmRequest.from = RF_MESH_NOADDRESS;
		mmRequest.dest = RF_MESH_BROADCAST;
		mmRequest.to = RF_MESH_BROADCAST;
		mmRequest.len = 0;
		for (;;) {
			if (SendAndWait(RF_MESH_PTYPE_WHOAMI_REQUEST, RF_MESH_PTYPE_WHOAMI_RESPONSE)) {
				hardwareConfig.hwType = mmResponse.data[0];
				hardwareConfig.hwVersion = mmResponse.data[1];
				for (uint8_t i = 0; i < 6; i++)
					hardwareConfig.macAddress[i] =  mmResponse.data[i + 2];
				hardwareConfig.crc = calcCRC(&hardwareConfig, sizeof hardwareConfig - 2);
				eeprom_write_block(&hardwareConfig, HARDWARE_CONFIG_ADDR, sizeof hardwareConfig);
				reboot();
			}
		}
	}
	wdt_reset();
	eeprom_read_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
	uint16_t rootNextHop = RF_MESH_NOADDRESS;
	if (calcCRC(&nodeConfig, sizeof nodeConfig) != 0) {
		nodeConfig.address = RF_MESH_NOADDRESS;
		TransceiverSetAddress(nodeConfig.address);
		mmRequest.source = RF_MESH_NOADDRESS;
		mmRequest.from = RF_MESH_NOADDRESS;
		mmRequest.dest = RF_MESH_BROADCAST;
		mmRequest.to = RF_MESH_BROADCAST;
		mmRequest.len = 6;
		for (uint8_t i = 0; i < 6; i++)
			mmRequest.data[i] = hardwareConfig.macAddress[i];
		while (nodeConfig.address == RF_MESH_NOADDRESS)
			if (SendAndWait(RF_MESH_PTYPE_DHCP_REQUEST, RF_MESH_PTYPE_DHCP_RESPONSE)) {
				uint8_t macCheck = 1;
				for (uint8_t m = 0; m < 6; m++)
					if (mmResponse.data[m] != hardwareConfig.macAddress[m]) {
						macCheck = 0;
						break;
					}
				if (macCheck > 0) {
					nodeConfig.address = (mmResponse.data[6 + 1] << 8) + mmResponse.data[6];
					nodeConfig.rootAddress = mmResponse.source;
					nodeConfig.fwVersion = 0;
					nodeConfig.fwBlockCount = 0;
					nodeConfig.fwCrc = 0;
					nodeConfig.configCrc = 0;
					nodeConfig.crc = calcCRC(&nodeConfig, sizeof nodeConfig - 2);
					eeprom_write_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
					rootNextHop = mmResponse.from;
				}
			}
	}
	wdt_reset();
	TransceiverSetAddress(nodeConfig.address);
	mmRequest.source = nodeConfig.address;
	mmRequest.from = nodeConfig.address;
	mmRequest.dest = nodeConfig.rootAddress;
	mmRequest.to = RF_MESH_BROADCAST;
	mmRequest.len = 0;
	for (uint8_t i = 0; i < 10; i++) {
		if (rootNextHop != RF_MESH_NOADDRESS)
			break;
		if (SendAndWait(RF_MESH_PTYPE_ARP_REQUEST, RF_MESH_PTYPE_ARP_RESPONSE))
			rootNextHop = mmResponse.from;
	}
	if (rootNextHop == RF_MESH_NOADDRESS) { // no response from root - start without announce
		startup();
	}
	wdt_reset();
	mmRequest.to = rootNextHop;
	mmRequest.len = 10; // ToDo : sizeof AnnounceRequest;
	AnnounceRequest *announceRequest = (AnnounceRequest *)mmRequest.data;
	announceRequest->hwType = hardwareConfig.hwType;
	announceRequest->hwVersion = hardwareConfig.hwVersion;
	announceRequest->fwVersion = nodeConfig.fwVersion;
	announceRequest->fwCrc = nodeConfig.fwCrc;
	announceRequest->configCrc = nodeConfig.configCrc;
	uint8_t announced = 0;
	for (uint8_t i = 0; i < 10; i++) {
		if (SendAndWait(RF_MESH_PTYPE_ANNOUNCE_REQUEST, RF_MESH_PTYPE_ANNOUNCE_RESPONSE)) {
			announced = 1;
			break;
		}
	}
	if (announced == 0) { // no response from root - start without announce
		startup();
	}
	AnnounceResponse *announceResponse = (AnnounceResponse *)mmResponse.data;
	if ((nodeConfig.fwVersion == announceResponse->fwVersion) && (nodeConfig.fwCrc == announceResponse->fwCrc) && validFirmware()) {
		if (nodeConfig.configCrc != announceResponse->configCrc) {
			nodeConfig.configCrc = 0;
			nodeConfig.crc = calcCRC(&nodeConfig, sizeof nodeConfig - 2);
			eeprom_write_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
		}
		startup();
	}
	led(LED_BITMASK_RED, 200);
	nodeConfig.fwVersion = announceResponse->fwVersion;
	nodeConfig.fwBlockCount = announceResponse->fwBlockCount;
	nodeConfig.fwCrc = announceResponse->fwCrc;
	nodeConfig.crc = calcCRC(&nodeConfig, sizeof nodeConfig - 2);
	mmRequest.len = 9; // ToDo : sizeof FirmwareRequest;
	FirmwareRequest *firmwareRequest = (FirmwareRequest *)mmRequest.data;
	FirmwareResponse *firmwareResponse = (FirmwareResponse *)mmResponse.data;
	firmwareRequest->hwType = hardwareConfig.hwType;
	firmwareRequest->hwVersion = hardwareConfig.hwVersion;
	firmwareRequest->fwVersion = nodeConfig.fwVersion;
	firmwareRequest->fwBlockSize = FIRMWARE_BLOCK_SIZE;
	wdt_reset();
	for (uint16_t block = 0; block < nodeConfig.fwBlockCount; block++) {
		firmwareRequest->fwBlockIndex = block;
		for (uint8_t i = 0; i < 101; i++) {
			if (SendAndWait(RF_MESH_PTYPE_FIRMWARE_REQUEST, RF_MESH_PTYPE_FIRMWARE_RESPONSE)) {
				if (block == firmwareResponse->fwBlockIndex) {
					uint8_t offset = (block * FIRMWARE_BLOCK_SIZE) % SPM_PAGESIZE;
					memcpy(progBuf + offset, firmwareResponse->fwBlock, FIRMWARE_BLOCK_SIZE);
					if (offset == SPM_PAGESIZE - FIRMWARE_BLOCK_SIZE)
						boot_program_page((block * FIRMWARE_BLOCK_SIZE) - offset, progBuf);
					break;
				} else if (i == 100) {
					reboot();
				}
			}
		}
	}
	wdt_reset();
	if (validFirmware())
		eeprom_write_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
}	

int main () {
	asm volatile ("clr __zero_reg__");
	// switch to 4 MHz
	clock_prescale_set(clock_div_4);
	MCUSR = 0;
//	wdt_disable();
	wdt_enable(WDTO_8S);
	LED_DDR |= LED_BITMASK_BOTH;
	led(LED_BITMASK_BOTH, 200);
/*
	uint8_t backoff = 0;
	while (run() > 100) {
		if (++backoff > 10)
			backoff = 0;
		clock_prescale_set(clock_div_256);
		for (long i = 0; i < 10000L << backoff && !dummy; ++i);
		clock_prescale_set(clock_div_4);
	}
	reboot();
*/
	run();
	reboot();
}
