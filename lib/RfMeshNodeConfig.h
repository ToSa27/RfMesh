#ifndef __RfMeshNodeConfig_h__
#define __RfMeshNodeConfig_h__

#include "RfMeshNodeType.h"

#ifdef RF_MESH_NODETYPE_MASTER
#define RF_MESH_HARDWARE_RASPBERRYPI
#define RF_MESH_MASTER_ADDRESS			0x0001
#include <arpa/inet.h>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <list>
#include <map>
#include <netinet/in.h>
#include <pthread.h>
#include <sstream>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <vector>
#include <wiringPi.h>
#include "RfMeshProtocol.h"
#include "RfMeshLogger.h"
#include "RfMeshUtils.h"
#include "RfMeshObserver.h"
#endif

#ifdef RF_MESH_NODETYPE_BOOTLOADER
#define RF_MESH_HARDWARE_ATMEGA328
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <string.h>
#include <util/crc16.h>
#include "RfMeshProtocol.h"
#endif

#ifdef RF_MESH_NODETYPE_LEAF
#define RF_MESH_HARDWARE_ATMEGA328
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/crc16.h>
#include "MiniArduino.h"
#include "RfMeshProtocol.h"
#endif

#ifdef RF_MESH_HARDWARE_RASPBERRYPI
#define	RFM70_PIN_SCLK		14
#define	RFM70_PIN_MOSI		12
#define	RFM70_PIN_MISO		13
#define	RFM70_PIN_RX_CSN	11
#define	RFM70_PIN_RX_CE		5
#define	RFM70_PIN_TX_CSN	10
#define	RFM70_PIN_TX_CE		6
#endif

#ifdef RF_MESH_HARDWARE_ATMEGA328
#define HARDWARE_CONFIG_ADDR	((uint8_t*) (1024 - 4 - 12)) // 1024 - 4 - sizeof HardwareConfig
#define NODE_CONFIG_ADDR		((uint8_t*) (1024 - 4 - 12 - 14)) // 1024 - 4 - sizeof HardwareConfig - sizeof NodeConfig
// arduino pin numbers
#define	RFM70_PIN_SCLK		13
#define	RFM70_PIN_MOSI		11
#define	RFM70_PIN_MISO		12
#define	RFM70_PIN_CSN		10
#define	RFM70_PIN_CE		8
#define	LED_PIN_RED			7
#define	LED_PIN_GREEN		6
// avr port and mask
#define RFM70_DDR			DDRB
#define RFM70_PORT			PORTB
#define RFM70_PIN			PINB
#define RFM70_BIT_OFFSET	8
#define	RFM70_BIT_SCLK		(RFM70_PIN_SCLK - RFM70_BIT_OFFSET)
#define	RFM70_BIT_MOSI		(RFM70_PIN_MOSI - RFM70_BIT_OFFSET)
#define	RFM70_BIT_MISO		(RFM70_PIN_MISO - RFM70_BIT_OFFSET)
#define	RFM70_BIT_CSN		(RFM70_PIN_CSN - RFM70_BIT_OFFSET)
#define	RFM70_BIT_CE		(RFM70_PIN_CE - RFM70_BIT_OFFSET)
#define LED_DDR				DDRD
#define LED_PORT			PORTD
#define LED_PIN				PIND
#define LED_BIT_OFFSET		0
#define LED_BIT_RED			(LED_PIN_RED - LED_BIT_OFFSET)
#define LED_BIT_GREEN		(LED_PIN_GREEN - LED_BIT_OFFSET)
#define LED_BITMASK_NONE	0
#define LED_BITMASK_RED		(1 << LED_BIT_RED)
#define LED_BITMASK_GREEN	(1 << LED_BIT_GREEN)
#define LED_BITMASK_BOTH	(LED_BITMASK_RED | LED_BITMASK_GREEN)
#endif

#endif
