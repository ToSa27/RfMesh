#ifndef __RfMeshNodeConfig_h__
#define __RfMeshNodeConfig_h__

#define RF_MESH_NODETYPE_LEAF

#define RF_MESH_HARDWARE_ATMEGA328
#define RF_MESH_DEFAULT_ADDRESS			0x0000

//#include "Arduino.h"
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdint.h>
#include <util/crc16.h>

//#define HARDWARE_CONFIG_ADDR	((uint8_t*) (1024 - 4 - sizeof HardwareConfig))
#define HARDWARE_CONFIG_ADDR	((uint8_t*) (1024 - 4 - 12))
//#define NODE_CONFIG_ADDR		((uint8_t*) (1024 - 4 - sizeof HardwareConfig - sizeof NodeConfig))
#define NODE_CONFIG_ADDR		((uint8_t*) (1024 - 4 - 12 - 14))

#include "common.h"

#endif
