#include "CapacitiveSensor.h"

#define CS_Timeout_Millis 620000
#define CS_AutocaL_Millis 20000

CapacitiveSensor::CapacitiveSensor(uint8_t sendPin, uint8_t receivePin)
{
	sPin = sendPin;
	rPin = receivePin;

	if (sendPin < 8)
		DDRD |= (1 << sendPin);
	else
		DDRB |= (1 << (sendPin - 8));

	leastTotal = 0x0FFFFFFFL;   // input large value for autocalibrate begin
	lastCal = millis();         // set millis for start
}

long CapacitiveSensor::capacitiveSensor(uint8_t samples)
{
	unsigned long  total = 0;

	for (uint8_t i = 0; i < samples; i++) {    // loop for samples parameter - simple lowpass filter
		if (sPin < 8) { // prereq: both pins on same port
			uint8_t sBit = (1 << sPin);
			uint8_t rBit = (1 << rPin);
			cli();
			PORTD &= ~sBit;        // set Send Pin Register low			
			DDRD &= ~rBit;        // set receivePin to input
			PORTD &= ~rBit;        // set receivePin Register low to make sure pullups are off
			DDRD |= rBit;         // set pin to OUTPUT - pin is now LOW AND OUTPUT
			DDRD &= ~rBit;        // set pin to INPUT 
			PORTD |= sBit;         // set send Pin High
			sei();
			while ( !(PIND & rBit)  && (total < CS_Timeout_Millis) )  // while receive pin is LOW AND total is positive value
				total++;
			if (total > CS_Timeout_Millis)
				return -2;         //  total variable over timeout
			// set receive pin HIGH briefly to charge up fully - because the while loop above will exit when pin is ~ 2.5V 
			cli();
			PORTD  |= rBit;        // set receive pin HIGH - turns on pullup 
			DDRD |= rBit;         // set pin to OUTPUT - pin is now HIGH AND OUTPUT
			DDRD &= ~rBit;        // set pin to INPUT 
			PORTD  &= ~rBit;       // turn off pullup
			PORTD &= ~sBit;        // set send Pin LOW
			sei();
			while ( (PIND & rBit) && (total < CS_Timeout_Millis) )  // while receive pin is HIGH  AND total is less than timeout
				total++;
			if (total >= CS_Timeout_Millis)
				return -2;     // total variable over timeout		
		} else if (sPin < 16) {
			uint8_t sBit = (1 << (sPin - 8));
			uint8_t rBit = (1 << (rPin - 8));
			cli();
			PORTB &= ~sBit;        // set Send Pin Register low			
			DDRB &= ~rBit;        // set receivePin to input
			PORTB &= ~rBit;        // set receivePin Register low to make sure pullups are off
			DDRB |= rBit;         // set pin to OUTPUT - pin is now LOW AND OUTPUT
			DDRB &= ~rBit;        // set pin to INPUT 
			PORTB |= sBit;         // set send Pin High
			sei();
			while ( !(PINB & rBit)  && (total < CS_Timeout_Millis) )  // while receive pin is LOW AND total is positive value
				total++;
			if (total > CS_Timeout_Millis)
				return -2;         //  total variable over timeout
			// set receive pin HIGH briefly to charge up fully - because the while loop above will exit when pin is ~ 2.5V 
			cli();
			PORTB  |= rBit;        // set receive pin HIGH - turns on pullup 
			DDRB |= rBit;         // set pin to OUTPUT - pin is now HIGH AND OUTPUT
			DDRB &= ~rBit;        // set pin to INPUT 
			PORTB  &= ~rBit;       // turn off pullup
			PORTB &= ~sBit;        // set send Pin LOW
			sei();
			while ( (PINB & rBit) && (total < CS_Timeout_Millis) )  // while receive pin is HIGH  AND total is less than timeout
				total++;
			if (total >= CS_Timeout_Millis)
				return -2;     // total variable over timeout		
		}
	}

	// only calibrate if time is greater than CS_AutocaL_Millis and total is less than 10% of baseline
	// this is an attempt to keep from calibrating when the sensor is seeing a "touched" signal
	if ( (millis() - lastCal > CS_AutocaL_Millis) && abs(total  - leastTotal) < (int)(.10 * (float)leastTotal) ) {
		leastTotal = 0x0FFFFFFFL;          // reset for "autocalibrate"
		lastCal = millis();
	}

	// routine to subtract baseline (non-sensed capacitance) from sensor return
	if (total < leastTotal)
		leastTotal = total;                 // set floor value to subtract from sensed value         
	return (total - leastTotal);
}

void CapacitiveSensor::reset_CS_AutoCal(void){
	leastTotal = 0x0FFFFFFFL;
}
