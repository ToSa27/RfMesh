#ifndef CapacitiveSensor_h
#define CapacitiveSensor_h

#include "RfMeshNodeConfig.h"

class CapacitiveSensor
{
  public:
	CapacitiveSensor(uint8_t sendPin, uint8_t receivePin);
	long capacitiveSensorRaw(uint8_t samples);
	long capacitiveSensor(uint8_t samples);
	void set_CS_Timeout_Millis(unsigned long timeout_millis);
	void reset_CS_AutoCal();
	void set_CS_AutocaL_Millis(unsigned long autoCal_millis);
  private:
	uint8_t sPin;
	uint8_t rPin;
	unsigned long  leastTotal;
	unsigned long  lastCal; // millis timestamp of last calibration
	int SenseOneCycle(void);
};

#endif
