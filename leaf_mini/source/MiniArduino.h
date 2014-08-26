#ifndef common_h
#define	common_h

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <stdlib.h>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#ifdef	__cplusplus
extern "C" {
#endif

extern void init(void);

// Start the clock - uses timer0.
extern void clock_start(void);

// Stop the clock.
//extern void clock_stop(void);

// Get the time since clock start, in milliseconds.
extern uint32_t millis(void);

extern void delay(unsigned long ms);

extern void delayMicroseconds(unsigned int us);

extern long rnd(long howbig);

extern void pinMode(uint8_t pin, uint8_t mode);

extern void digitalWrite(uint8_t pin, uint8_t val);

extern int digitalRead(uint8_t pin);

#ifdef	__cplusplus
}
#endif

#endif
 
