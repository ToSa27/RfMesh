#if F_CPU == 8000000UL
#define OCR0A_SCALE 125    // 8MHz / 64 / 125 = 1KHz.
#elif F_CPU == 16000000UL
#define OCR0A_SCALE 250    // 16MHz / 64 / 250 = 1KHz.
#endif

#include "common.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint32_t milli_count = 0;

ISR(TIMER0_COMPA_vect)
{
    milli_count++;
}

/*
 * Start the clock - uses timer0.
 */
void clock_start(void)
{
    // Disable counter 0 - set prescaler to 0.
    TCCR0B = 0;

    // Initialize counter 0.
    milli_count = 0;
    TCCR0A = BIT(WGM01);     // CTC mode.
    TCNT0 = 0;               // Start from 0.
    OCR0A = OCR0A_SCALE;
    OCR0B = 0;               // Unused.
    TIMSK0 = BIT(OCIE0A);    // Enable interrupt on OCR0A compare match.

    // Start counter 0 - set prescaler to CLK/64.
    TCCR0B = BIT(CS01) | BIT(CS00);
}

/*
 * Stop the clock.
 */
void clock_stop(void)
{
    // Disable counter0 - set prescaler to 0.
    TCCR0B = 0;
}

/*
 * Get the time since clock start, in milliseconds.
 */
uint32_t millis(void)
{
    uint8_t sreg = SREG;
    cli();
    uint32_t mc = milli_count;
    SREG = sreg;
    return mc;
}

void delay(unsigned long ms)
{
	uint32_t start = millis();
	while (start + ms > millis()) { }
}

void delayMicroseconds(unsigned int us)
{
#if F_CPU >= 16000000L
	if (--us == 0)
		return;
	us <<= 2;
	us -= 2;
#else
	if (--us == 0)
		return;
	if (--us == 0)
		return;
	us <<= 1;
	us--;
#endif
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
}


long rnd(long howbig)
{
  if (howbig == 0) {
    return 0;
  }
  return random() % howbig;
}


