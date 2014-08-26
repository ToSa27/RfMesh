#ifndef common_h
#define	common_h

#include "RfMeshNodeConfig.h"

//#define BIT(B)              (1 << (uint8_t)(B))       // Bit number.
//#define GET_BIT(V, B)       ((V) & (uint8_t)BIT(B))   // Get bit number.
//#define GET_BIT_HI(V, B)    ((V) | (uint8_t)BIT(B))   // Get with bit number hi.
//#define GET_BIT_LO(V, B)    ((V) & (uint8_t)~BIT(B))  // Get with bit number lo.
//#define SET_BIT_HI(V, B)    (V) |= (uint8_t)BIT(B)    // Set bit number hi.
//#define SET_BIT_LO(V, B)    (V) &= (uint8_t)~BIT(B)   // Set bit number lo.
//#define MASK(V, M)          ((V) & (uint8_t)(M))      // Mask bits.
//#define GET_MASK(V, M)      ((V) & (uint8_t)(M))      // Get mask bits.
//#define GET_MASK_HI(V, M)   ((V) | (uint8_t)(M))      // Get with mask bits hi.
//#define GET_MASK_LO(V, M)   ((V) & (uint8_t)~(M))     // Get with mask bits lo.
//#define SET_MASK_HI(V, M)   (V) |= (uint8_t)(M)       // Set mask bits hi.
//#define SET_MASK_LO(V, M)   (V) &= (uint8_t)~(M)      // Set mask bits lo.

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#ifdef	__cplusplus
extern "C" {
#endif

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
 
