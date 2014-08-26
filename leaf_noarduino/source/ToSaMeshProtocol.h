#ifndef __RfMeshProtocol_h__
#define __RfMeshProtocol_h__

#define RF_MESH_NOADDRESS						0x0000
#define RF_MESH_BROADCAST						0xFFFF
#define RF_MESH_HOP_RETRY						5

#define RF_MESH_RAW_MESSAGE_LEN_MAX			32
#define RF_MESH_MESH_MESSAGE_LEN_HEADER		9
#define RF_MESH_MESH_MESSAGE_LEN_MAX			RF_MESH_RAW_MESSAGE_LEN_MAX - RF_MESH_MESH_MESSAGE_LEN_HEADER
#define RF_MESH_APP_MESSAGE_LEN_MAX			RF_MESH_MESH_MESSAGE_LEN_MAX

/* package types with high bit 7 set (& 0x80 > 0) require a hopack */
/* package types with high bit 6 set (& 0x40 > 0) require an ack */
#define RF_MESH_PTYPE_UNKNOWN					0x00	/* ignore */
#define RF_MESH_PTYPE_HOPACK					0x01	/* no hopack / no ack */
#define RF_MESH_PTYPE_WHOAMI_REQUEST			0x02	/* no hopack / no ack */
#define RF_MESH_PTYPE_ARP_REQUEST				0x03	/* no hopack / no ack */
#define RF_MESH_PTYPE_DHCP_REQUEST			0x04	/* no hopack / no ack */
#define RF_MESH_PTYPE_TIME_REQUEST			0x05	/* no hopack / no ack */
#define RF_MESH_PTYPE_TIME_BROADCAST			0x06	/* no hopack / no ack */
#define RF_MESH_PTYPE_HOLIDAY_REQUEST			0x07	/* no hopack / no ack */
#define RF_MESH_PTYPE_HOLIDAY_BROADCAST		0x08	/* no hopack / no ack */
#define RF_MESH_PTYPE_HEARTBEAT				0x09	/* no hopack / no ack */
#define RF_MESH_PTYPE_ACK						0x80	/* hopack / no ack */
#define RF_MESH_PTYPE_NACK					0x81	/* hopack / no ack */
#define RF_MESH_PTYPE_WHOAMI_RESPONSE			0x82	/* hopack / no ack */
#define RF_MESH_PTYPE_ARP_RESPONSE			0x83	/* hopack / no ack */
#define RF_MESH_PTYPE_ARP_FAILURE				0x84	/* hopack / no ack */
#define RF_MESH_PTYPE_DHCP_RESPONSE			0x85	/* hopack / no ack */
#define RF_MESH_PTYPE_ANNOUNCE_REQUEST		0x86	/* hopack / no ack */
#define RF_MESH_PTYPE_ANNOUNCE_RESPONSE		0x87	/* hopack / no ack */
#define RF_MESH_PTYPE_FIRMWARE_REQUEST		0x88	/* hopack / no ack */
#define RF_MESH_PTYPE_FIRMWARE_RESPONSE		0x89	/* hopack / no ack */
#define RF_MESH_PTYPE_CONFIG_REQUEST			0x8A	/* hopack / no ack */
#define RF_MESH_PTYPE_CONFIG_RESPONSE			0x8B	/* hopack / no ack */
#define RF_MESH_PTYPE_DEBUG					0x8C	/* hopack / no ack */
#define RF_MESH_PTYPE_PING					0x8D	/* hopack / no ack */
#define RF_MESH_PTYPE_PONG					0x8E	/* hopack / no ack */

//#define RF_MESH_PTYPE_MANUAL_SET				0xD0	/* hopack / ack */
//#define RF_MESH_PTYPE_MANUAL_GET_REQUEST		0x91	/* hopack / no ack */
//#define RF_MESH_PTYPE_MANUAL_GET_RESPONSE		0xD1	/* hopack / ack */
//#define RF_MESH_PTYPE_TIMER_SET				0xD2	/* hopack / ack */
//#define RF_MESH_PTYPE_TIMER_GET_REQUEST		0x93	/* hopack / no ack */
//#define RF_MESH_PTYPE_TIMER_GET_RESPONSE		0xD3	/* hopack / ack */
//#define RF_MESH_PTYPE_STATE_SET				0xD4	/* hopack / ack */
//#define RF_MESH_PTYPE_STATE_GET_REQUEST		0x95	/* hopack / no ack */
//#define RF_MESH_PTYPE_STATE_GET_RESPONSE		0xD5	/* hopack / ack */
//#define RF_MESH_PTYPE_STATE_SELECT			0xD6	/* hopack / ack */

#define RF_MESH_PTYPE_DATASYNC				0xF0	/* hopack / ack */
#define RF_MESH_PTYPE_DATA_SET				0xF1	/* hopack / ack */
#define RF_MESH_PTYPE_DATA_GET_REQUEST		0xB2	/* hopack / no ack */
#define RF_MESH_PTYPE_DATA_GET_RESPONSE		0xF2	/* hopack / ack */

#define RF_MESH_CONFIG_FETCHALL			0x80
#define RF_MESH_CONFIG_TYPE_HEADER		0x00
#define RF_MESH_CONFIG_TYPE_IO			0x01
#define RF_MESH_CONFIG_TYPE_NETWORK		0x02
#define RF_MESH_CONFIG_TYPE_CONTROL		0x03
#define RF_MESH_CONFIG_TYPE_PARAMETER		0x04
#define RF_MESH_CONFIG_TYPE_TIMER			0x05
#define RF_MESH_CONFIG_TYPE_LAST			RF_MESH_CONFIG_TYPE_TIMER

#define FIRMWARE_BLOCK_SIZE		0x10

#include <stdint.h>

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */

typedef struct {
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
} meshMessage;

typedef struct {
    uint16_t source;
    uint16_t dest;
    uint8_t pid;
    uint8_t ptype;
    uint8_t len;
	uint8_t data[RF_MESH_APP_MESSAGE_LEN_MAX];
} appMessage;

typedef struct {
	uint8_t macAddress[6];
} DhcpRequest;

typedef struct {
	uint8_t macAddress[6];
	uint16_t newAddress;
} DhcpResponse;

typedef struct {
	uint16_t hwType;
	uint16_t hwVersion;
	uint16_t fwVersion;
	uint16_t fwCrc;
	uint16_t configCrc;
} AnnounceRequest;

typedef struct {
	uint16_t fwVersion;
	uint16_t fwBlockCount;
	uint16_t fwCrc;
	uint16_t configCrc;
} AnnounceResponse;

typedef struct {
	uint16_t hwType;
	uint16_t hwVersion;
	uint16_t fwVersion;
	uint8_t fwBlockSize;
	uint16_t fwBlockIndex;
} FirmwareRequest;

typedef struct {
	uint16_t fwBlockIndex;
	uint8_t fwBlock[FIRMWARE_BLOCK_SIZE];
} FirmwareResponse;


typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
} Datestamp;

typedef struct {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t weekday;
} Timestamp;

struct HardwareConfig {
	uint16_t hwType;
	uint16_t hwVersion;
	uint8_t macAddress[6];
	uint16_t crc;
};

struct NodeConfig {
	uint16_t address;
	uint16_t rootAddress;
	uint16_t fwVersion;
	uint16_t fwBlockCount;
	uint16_t fwCrc;
	uint16_t configCrc;
	uint16_t crc;
};

#define EVENTDATALEN	4

#define RF_MESH_ETYPE_DIGITALWRITE	0x01	// first byte is pin address, second byte is value 0 or 1
#define RF_MESH_ETYPE_DIGITALREAD		0x02
#define RF_MESH_ETYPE_ANALOGWRITE		0x03
#define RF_MESH_ETYPE_ANALOGREAD		0x04


#pragma pack(pop)   /* restore original alignment from stack */

#endif