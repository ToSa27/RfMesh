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

#include "MiniArduino.h"

#define	LED_PIN_RED			7
#define	LED_PIN_GREEN		6
#define LED_DDR				DDRD
#define LED_PORT			PORTD
#define LED_BIT_OFFSET		0
#define LED_BIT_RED			(LED_PIN_RED - LED_BIT_OFFSET)
#define LED_BIT_GREEN		(LED_PIN_GREEN - LED_BIT_OFFSET)
#define LED_BITMASK_NONE	0
#define LED_BITMASK_RED		(1 << LED_BIT_RED)
#define LED_BITMASK_GREEN	(1 << LED_BIT_GREEN)
#define LED_BITMASK_BOTH	(LED_BITMASK_RED | LED_BITMASK_GREEN)

static void led(uint8_t mask, uint16_t t) {
	LED_PORT = (LED_PORT & ~(LED_BITMASK_BOTH)) | mask;
	if (t > 0) {
		delay(t);
		LED_PORT = (LED_PORT & ~(LED_BITMASK_BOTH));
		delay(t);
	}
}

int main () {
	init();
	clock_start();
	LED_DDR |= LED_BITMASK_BOTH;
	for (;;) {
		led(LED_BITMASK_RED, 200);
		led(LED_BITMASK_GREEN, 200);
	}
}
