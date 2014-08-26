#ifndef __RfMeshLeafGP_h__
#define __RfMeshLeafGP_h__

#include "RfMeshLeafBase.h"

#define CONFIG_ADDR 		((uint8_t*) 0)

#define MEM_BIT_COUNT		16 // must be 8*x
#define MEM_BYTE_COUNT		16
#define MEM_WORD_COUNT		16
#define MEM_COUNT			MEM_BIT_COUNT + MEM_BYTE_COUNT + MEM_WORD_COUNT // 48

#define MEM_BASE_MIN		0
#define MEM_BIT_MIN			MEM_BASE_MIN // 0
#define MEM_BIT_MAX			MEM_BIT_MIN + MEM_BIT_COUNT - 1 // 15
#define MEM_BYTE_MIN		MEM_BIT_MAX + 1 // 16
#define MEM_BYTE_MAX		MEM_BYTE_MIN + MEM_BYTE_COUNT - 1 // 31
#define MEM_WORD_MIN		MEM_BYTE_MAX + 1 // 32
#define MEM_WORD_MAX		MEM_WORD_MIN + MEM_WORD_COUNT - 1 // 47
#define MEM_BASE_MAX		MEM_WORD_MAX // 47

#define MEM_BASE_ADDR_MIN	0
#define MEM_BIT_ADDR_MIN	MEM_BASE_ADDR_MIN // 0
#define MEM_BIT_ADDR_MAX	MEM_BIT_ADDR_MIN + (MEM_BIT_COUNT / 8) - 1 // 1
#define MEM_BYTE_ADDR_MIN	MEM_BIT_ADDR_MAX + 1 // 2
#define MEM_BYTE_ADDR_MAX	MEM_BYTE_ADDR_MIN + (MEM_BYTE_COUNT * 1) - 1 // 17
#define MEM_WORD_ADDR_MIN	MEM_BYTE_ADDR_MAX + 1 // 18
#define MEM_WORD_ADDR_MAX	MEM_WORD_ADDR_MIN + (MEM_WORD_COUNT * 2) - 1 // 49
#define MEM_BASE_ADDR_MAX	MEM_WORD_ADDR_MAX // 49

#define MEM_NONE			255

#define MEM_DATA_COUNT		MEM_BASE_ADDR_MAX - MEM_BASE_ADDR_MIN + 1
#define MEM_FLAG_COUNT		6 // roundup((MEM_BASE_MAX - MEM_BASE_MIN + 1) / 8)

#define CONFIG_MAX_SIZE		512

#define PROTO_IO_NONE						0x00
#define PROTO_IO_D_IN_BUTTON_ACTIVE_LOW		0xC0
#define PROTO_IO_D_IN_BUTTON_ACTIVE_LOW_PU	0xE0	// internal pull-up
#define PROTO_IO_D_IN_BUTTON_ACTIVE_HIGH	0xC1
#define PROTO_IO_D_IN_BUTTON_ACTIVE_HIGH_PU	0xE1	// internal pull-up
#define PROTO_IO_D_IN_SWITCH				0xC2
#define PROTO_IO_D_IN_SWITCH_PU				0xE2	// internal pull-up
#define PROTO_IO_D_OUT_LED_DL				0x40	// default low
#define PROTO_IO_D_OUT_LED_DH				0x60	// default high
#define PROTO_IO_D_OUT_220_DL				0x41	// default low
#define PROTO_IO_D_OUT_220_DH				0x61	// default high
#define PROTO_CONTROL_NONE					0x0000
#define PROTO_CONTROL_ONOFF					0x0001
#define PARAM_CONTROL_ONOFF_MO				0x00
#define PARAM_CONTROL_ONOFF_MO_INV			0x01
#define PARAM_CONTROL_ONOFF_MI_TOGGLE		0x02
#define PARAM_CONTROL_ONOFF_MI_SET			0x03
#define PROTO_CONTROL_HEATING				0x0002
#define PARAM_CONTROL_HEATING_MEM_STATUS	0x00	// byte: MSB: frost due to window open / two LSBs current status
#define PARAM_CONTROL_HEATING_VAL_FROST		0x01	// byte: target temperature frost
#define PARAM_CONTROL_HEATING_VAL_NIGHT		0x02	// byte: target temperature night
#define PARAM_CONTROL_HEATING_VAL_DAY		0x03	// byte: target temperature day
#define PARAM_CONTROL_HEATING_VAL_OFFSET	0x04	// byte: sensor offset
#define PARAM_CONTROL_HEATING_SENSOR		0x05	// byte: sensor input
#define PARAM_CONTROL_HEATING_NEXT			0x06	// bit: button input 
#define PARAM_CONTROL_HEATING_SET			0x07	// byte: set to new state
#define PARAM_CONTROL_HEATING_WINDOW		0x08	// bit: window open sensor(s)
#define PARAM_CONTROL_HEATING_VALVE			0x09	// bit: valve on/off
#define PARAM_CONTROL_HEATING_IS_FROST		0x0A	// bit: led snow
#define PARAM_CONTROL_HEATING_IS_NIGHT		0x0B	// bit: led moon
#define PARAM_CONTROL_HEATING_IS_DAY		0x0C	// bit: led sun
#define PROTO_CONTROL_BLIND					0x0003
#define PARAM_CONTROL_BLIND_MEM_TIMER		0x00
#define PARAM_CONTROL_BLIND_MEM_POSITION	0x01
#define PARAM_CONTROL_BLIND_SUN_POSITION	0x02
#define PARAM_CONTROL_BLIND_MI_UP			0x03
#define PARAM_CONTROL_BLIND_MI_DOWN			0x04
#define PARAM_CONTROL_BLIND_MI_SET_AUTO		0x05
#define PARAM_CONTROL_BLIND_MI_SUN			0x06
#define PARAM_CONTROL_BLIND_MI_WINDOW		0x07
#define PARAM_CONTROL_BLIND_MI_AUTO			0x08
#define PARAM_CONTROL_BLIND_MO_AUTO			0x09
#define PARAM_CONTROL_BLIND_MO_ENGINE		0x0A
#define PARAM_CONTROL_BLIND_MO_DIRECTION	0x0B
#define PROTO_NETWORK_NONE					0x00
#define PROTO_NETWORK_TX					0x01
#define PROTO_NETWORK_RX					0x81
#define PROTO_NETWORK_RX_BRDCST				0xC1
#define PROTO_TIMER_NONE					0x00
#define PROTO_TIMER_SETVALUE				0x01

struct ConfigHeader {
	uint8_t ioCount; 			// 3 bytes each
	uint8_t networkCount;		// 5 bytes each
	uint8_t controlCount;		// 5 bytes each
	uint16_t parameterCount;	// 2 bytes each
	uint8_t timerCount;			// 6 bytes each
	uint16_t crc;
};

// 1a / 1b / 5a / 5b
typedef struct IOConfig {
	uint8_t ioPrototype; // MSB = 0:output / 1:input
	                     // 2nd MSB = 0:analogue / 1:digital
	                     // 3rd MSB
						 //   if digital input:
						 //     0:internal pull-up disabled / 1:internal pull-up enabled
						 //   if digital output:
						 //     0:default output low / 1:default output high
	uint8_t pinNumber;
	uint8_t memAddress;
};

#define RF_MESH_CONFIG_TYPE_IO_SIZE			3

// 2b / 4
typedef struct NetworkConfig {
	uint8_t networkPrototype; // MSB = 0:tx / 1:rx
	                          // 2nd MSB : toBroadcast
							  //           rx: if 0 then only accept if package dest is own address
							  //               if 1 then accept if package dest is own address or broadcast
							  //           tx: n/a
	uint16_t address;
	uint8_t sourceMemAddress;
	uint8_t destMemAddress;
};

#define RF_MESH_CONFIG_TYPE_NETWORK_SIZE		5

// 3
typedef struct ControlConfig {
	uint16_t controlPrototype;
	uint8_t paramCount;
	uint16_t paramStart;
};

#define RF_MESH_CONFIG_TYPE_CONTROL_SIZE		5

typedef struct ParameterConfig {
	uint8_t type;
	uint8_t value;
};

#define RF_MESH_CONFIG_TYPE_PARAMETER_SIZE	2

// 2a
typedef struct TimerConfig {
	uint8_t timerPrototype;
	uint8_t hour;		// lower 5 bits -> hour 0-23 / higher 3 bits -> random 0-7 (x5 min)
	uint8_t minute;		// lower 6 bits -> minute 0-59 / highest bit -> vacation / second highest bit -> bank holiday
	uint8_t weekdays;
//	uint8_t memAddress;
//	uint8_t value;	
	uint8_t paramCount;
	uint16_t paramStart;
};

#define RF_MESH_CONFIG_TYPE_TIMER_SIZE		7

class RfMeshLeaf : public RfMeshLeafBase {
	public:
		void setup();
		void loop();
	private:
		bool blinkState;
		unsigned long blinkNext;
		bool isBankHoliday();
		bool isVacation();
		uint8_t delayedExecTimer;
		unsigned long delayedExecMillis;
		void execTimer(uint8_t timer);
		uint8_t changeFlag[MEM_FLAG_COUNT];
		void resetFlags();
		bool isFlagged(uint8_t address);
		uint8_t memData[MEM_DATA_COUNT];
		uint8_t getValue(uint8_t address);
		void setValue(uint8_t address, uint8_t value, uint8_t setFlag);
//		IOConfig ioConfig[CONFIG_MAX_IO];
//		TimerConfig timerConfig[CONFIG_MAX_TIMER];
//		NetworkConfig networkConfig[CONFIG_MAX_NETWORK];
//		ControlConfig controlConfig[CONFIG_MAX_CONTROL];
		bool validConfig;
		void setupIO();
		struct ConfigHeader configHeader;
		void rxConfig(appMessage am);
		uint16_t getConfigSize();
		uint8_t configData[CONFIG_MAX_SIZE];
		IOConfig * ioConfig;
		NetworkConfig * networkConfig;
		ControlConfig * controlConfig;
		ParameterConfig * parameterConfig;
		TimerConfig * timerConfig;
		uint8_t SendAndWaitForConfig(uint8_t configType, uint8_t configId);
		void sendDebug(uint8_t position, uint8_t data1, uint8_t data2);
		void organizeConfigMem();
		void loadConfig();
		void saveConfig();
		void fetchConfig();
		void checkInputs();
		void checkTimer();
		void rxNetwork();
		void execControls();
		void txNetwork();
		void setOutputs();
		
		uint8_t getParam(uint8_t type, ParameterConfig * pc, uint8_t count);

		void execControl_OnOff(uint8_t ci);
		void execControl_Heating(uint8_t ci);
		void execControl_Blind(uint8_t ci);
};

#endif
