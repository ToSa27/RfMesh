#if F_CPU == 8000000UL
#define OCR0A_SCALE 125    // 8MHz / 64 / 125 = 1KHz.
#elif F_CPU == 16000000UL
#define OCR0A_SCALE 250    // 16MHz / 64 / 250 = 1KHz.
#endif

#include "common.h"

static volatile uint32_t milli_count = 0;

ISR(TIMER0_COMPA_vect) {
    milli_count++;
}

/*
 * Start the clock - uses timer0.
 */
void clock_start(void) {
    // Disable counter 0 - set prescaler to 0.
    TCCR0B = 0;

    // Initialize counter 0.
    milli_count = 0;
    TCCR0A = 1 << WGM01;     // CTC mode.
    TCNT0 = 0;               // Start from 0.
    OCR0A = OCR0A_SCALE;
    OCR0B = 0;               // Unused.
    TIMSK0 = 1 << OCIE0A;    // Enable interrupt on OCR0A compare match.

    // Start counter 0 - set prescaler to CLK/64.
    TCCR0B = (1 << CS01) | (1 << CS00);
}

/*
 * Stop the clock.
 */
//void clock_stop(void) {
    // Disable counter0 - set prescaler to 0.
//    TCCR0B = 0;
//}

/*
 * Get the time since clock start, in milliseconds.
 */
uint32_t millis(void) {
    uint8_t sreg = SREG;
    cli();
    uint32_t mc = milli_count;
    SREG = sreg;
    return mc;
}

void delay(unsigned long ms) {
	uint32_t start = millis();
	while (start + ms > millis()) { }
}

void delayMicroseconds(unsigned int us) {
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


long rnd(long howbig) {
	if (howbig == 0) {
		return 0;
	}
	return random() % howbig;
}

void pinMode(uint8_t pin, uint8_t mode) {
	if (pin < 8) { // D
		uint8_t bit = (1 << pin);
		if (mode == INPUT) { 
			DDRD &= ~bit;
		} else if (mode == INPUT_PULLUP) {
			DDRD &= ~bit;
			PORTD |= bit;
		} else {
			DDRD |= bit;
		}
	} else if (pin < 16) { // B
		uint8_t bit = (1 << (pin - 8));
		if (mode == INPUT) { 
			DDRB &= ~bit;
		} else if (mode == INPUT_PULLUP) {
			DDRB &= ~bit;
			PORTB |= bit;
		} else {
			DDRB |= bit;
		}
	}
}

void digitalWrite(uint8_t pin, uint8_t val) {
	if (pin < 8) { // D
		uint8_t bit = (1 << pin);
		if (val == LOW)
			PORTD &= ~bit;
		else
			PORTD |= bit;
	} else if (pin < 16) { // B
		uint8_t bit = (1 << (pin - 8));
		if (val == LOW)
			PORTB &= ~bit;
		else
			PORTB |= bit;
	}
}

int digitalRead(uint8_t pin) {
	if (pin < 8) { // D
		uint8_t bit = (1 << pin);
		if (PIND & bit)
			return HIGH;
		return LOW;
	} else if (pin < 16) { // B
		uint8_t bit = (1 << (pin - 8));
		if (PINB & bit)
			return HIGH;
		return LOW;
	}
	return 255;
}
