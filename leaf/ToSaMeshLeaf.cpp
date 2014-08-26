#include "RfMeshLeaf.h"

int main(void) {
	init();
	clock_start();
	wdt_enable(WDTO_8S);
	RfMeshLeaf leaf;
	leaf.init();
	leaf.setup();
	for (;;) {
		wdt_reset();
		leaf.loop();
	}
	return 0;
}

void RfMeshLeaf::setup() {
	blinkState = 0x01;
	pinMode(LED_PIN_RED, OUTPUT);
	digitalWrite(LED_PIN_RED, blinkState);
	blinkNext = millis() + 1000;
	delayedExecTimer = 255;
	lastTimerExecMin = 255;
	validConfig = false;
	onewire_next = 0;
	capsense_next = millis() + 100;
//	lcd.begin(40, 2);
//	lcd.print("RfMesh");
	loadConfig();
}

void RfMeshLeaf::loop() {
	wdt_reset();
	node->TickTx();
	if (millis() > blinkNext) {
		blinkState ^= 0x01;
		digitalWrite(LED_PIN_RED, blinkState);
		blinkNext = millis() + 1000;
	}
	resetFlags();
	if (validConfig) {
		wdt_reset();
		checkInputs();
		checkTimer();
	}
	wdt_reset();
	rxNetwork();
	if (validConfig) {
		wdt_reset();
		execControls();
		wdt_reset();
		txNetwork();
		setOutputs();
	}
}

bool RfMeshLeaf::isBankHoliday() {
	if (node->tsLast.year == node->nextBankHoliday.year)
		if (node->tsLast.month == node->nextBankHoliday.month)
			if (node->tsLast.day == node->nextBankHoliday.day)
				return true;
	return false;
}

bool RfMeshLeaf::isVacation() {
	bool started = false;
	if (node->tsLast.year > node->nextVacationStart.year) {
		started = true;
	} else if (node->tsLast.year == node->nextVacationStart.year) {
		if (node->tsLast.month > node->nextVacationStart.month) {
			started = true;
		} else if (node->tsLast.month == node->nextVacationStart.month) {
			if (node->tsLast.day >= node->nextVacationStart.day)
				started = true;
		}		
	}
	if (started) {
		if (node->tsLast.year < node->nextVacationEnd.year) {
			return true;
		} else if (node->tsLast.year == node->nextVacationEnd.year) {
			if (node->tsLast.month < node->nextVacationEnd.month) {
				return true;
			} else if (node->tsLast.month == node->nextVacationEnd.month) {
				if (node->tsLast.day <= node->nextVacationEnd.day) {
					return true;
				}
			}
		}
	}
	return false;
}

void RfMeshLeaf::resetFlags() {
	for (uint8_t i = 0; i < MEM_FLAG_COUNT; i++)
		changeFlag[i] = 0;
}

bool RfMeshLeaf::isFlagged(uint8_t address) {
	return ((changeFlag[address >> 3] & (1 << (address & 0x03))) > 0);
}

uint16_t RfMeshLeaf::getValue(uint8_t address) {
	if (address <= MEM_BIT_MAX) {
		if ((memData[address >> 3] & (1 << (address & 0x03))) > 0)
			return 1;
		return 0;
	} else if (address <= MEM_BYTE_MAX)
		return memData[MEM_BYTE_ADDR_MIN + address - MEM_BYTE_MIN];
	else if (address <= MEM_WORD_MAX)
		return (memData[MEM_WORD_ADDR_MIN + 2 * (address - MEM_WORD_MIN) + 1] << 8) + memData[MEM_WORD_ADDR_MIN + 2 * (address - MEM_WORD_MIN)];
	return 0;
}

void RfMeshLeaf::setValue(uint8_t address, uint16_t value, uint8_t setFlag) {
	if (getValue(address) != value) {
		if (address <= MEM_BIT_MAX) {
			if (value == 0)
				memData[address >> 3] &= ((1 << (address & 0x03)) ^ 0xFF);
			else
				memData[address >> 3] |= (1 << (address & 0x03));
		} else if (address <= MEM_BYTE_MAX) {
			memData[MEM_BYTE_ADDR_MIN + address - MEM_BYTE_MIN] = value & 0x00FF;
		} else if (address <= MEM_WORD_MAX) {
			memData[MEM_WORD_ADDR_MIN + 2 * (address - MEM_WORD_MIN) + 1] = ((value & 0xFF00) >> 8);
			memData[MEM_WORD_ADDR_MIN + 2 * (address - MEM_WORD_MIN)] = (value & 0x00FF);
		}
		if (setFlag > 0)
			changeFlag[address >> 3] |= (1 << (address & 0x03));
	} else if (setFlag > 1)
		changeFlag[address >> 3] |= (1 << (address & 0x03));
}

void RfMeshLeaf::rxConfig(appMessage am) {
	nodeConfig.configCrc = 0; // invalidate crc unless recreated after save
	uint8_t configType = am.data[0];
	uint8_t maskedConfigType = am.data[0] & 0x7F; // !RF_MESH_CONFIG_FETCHALL;
	uint8_t configId = am.data[1];
	switch (maskedConfigType) {
		case RF_MESH_CONFIG_TYPE_HEADER:
			configHeader.ioCount = am.data[2];
			configHeader.networkCount = am.data[3];
			configHeader.controlCount = am.data[4];
			configHeader.parameterCount = (am.data[6] << 8) + am.data[5];
			configHeader.timerCount = am.data[7];
			configHeader.crc = calcCRC(&configHeader, sizeof configHeader - 2);
			organizeConfigMem();
			break;
		case RF_MESH_CONFIG_TYPE_IO:
			ioConfig[configId].ioPrototype = am.data[2];
			ioConfig[configId].pinNumber = am.data[3];
			ioConfig[configId].memAddress = am.data[4];
			break;
		case RF_MESH_CONFIG_TYPE_NETWORK:
			networkConfig[configId].networkPrototype = am.data[2];
			networkConfig[configId].address = (am.data[4] << 8) + am.data[3];
			networkConfig[configId].sourceMemAddress = am.data[5];
			networkConfig[configId].destMemAddress = am.data[6];
			break;
		case RF_MESH_CONFIG_TYPE_CONTROL:
			controlConfig[configId].controlPrototype = (am.data[3] << 8) + am.data[2];
			controlConfig[configId].paramCount = am.data[4];
			controlConfig[configId].paramStart = (am.data[6] << 8) + am.data[5];
			break;
		case RF_MESH_CONFIG_TYPE_PARAMETER:
			parameterConfig[configId].type = am.data[2];
			parameterConfig[configId].value = am.data[3];
			break;
		case RF_MESH_CONFIG_TYPE_TIMER:
			timerConfig[configId].timerPrototype = am.data[2];
			timerConfig[configId].hour = am.data[3];
			timerConfig[configId].minute = am.data[4];
			timerConfig[configId].weekdays = am.data[5];
			timerConfig[configId].paramCount = am.data[6];
			timerConfig[configId].paramStart = (am.data[8] << 8) + am.data[7];
			break;
	}
	if ((configType & RF_MESH_CONFIG_FETCHALL) > 0) {
		appMessage amr;
		amr.source = node->addr;
		amr.dest = nodeConfig.rootAddress;
		amr.pid = 0;
		amr.ptype = RF_MESH_PTYPE_CONFIG_REQUEST;
		amr.len = 2;
		amr.data[0] = maskedConfigType;
		amr.data[1] = configId + 1;
		bool doFetch = false;
		for (;;) {
			switch (amr.data[0]) {
				case RF_MESH_CONFIG_TYPE_IO:
					if (configHeader.ioCount > amr.data[1])
						doFetch = true;
					break;
				case RF_MESH_CONFIG_TYPE_NETWORK:
					if (configHeader.networkCount > amr.data[1])
						doFetch = true;
					break;
				case RF_MESH_CONFIG_TYPE_CONTROL:
					if (configHeader.controlCount > amr.data[1])
						doFetch = true;
					break;
				case RF_MESH_CONFIG_TYPE_PARAMETER:
					if (configHeader.parameterCount > amr.data[1])
						doFetch = true;
					break;
				case RF_MESH_CONFIG_TYPE_TIMER:
					if (configHeader.timerCount > amr.data[1])
						doFetch = true;
					break;
			}
			if (doFetch) {
				amr.data[0] |= RF_MESH_CONFIG_FETCHALL;
				node->Send(amr);
				return;
			} else {
				amr.data[0] += 1;
				amr.data[1] = 0;
				if (amr.data[0] > RF_MESH_CONFIG_TYPE_LAST) {
					saveConfig();
					return;
				}
			}
		}
	}
	saveConfig();
}

void RfMeshLeaf::organizeConfigMem() {
	uint8_t* configPos = configData;
	ioConfig = (IOConfig *)configPos;
	configPos += configHeader.ioCount * RF_MESH_CONFIG_TYPE_IO_SIZE;
	networkConfig = (NetworkConfig *)configPos;
	configPos += configHeader.networkCount * RF_MESH_CONFIG_TYPE_NETWORK_SIZE;
	controlConfig = (ControlConfig *)configPos;
	configPos += configHeader.controlCount * RF_MESH_CONFIG_TYPE_CONTROL_SIZE;
	parameterConfig = (ParameterConfig *)configPos;
	configPos += configHeader.parameterCount * RF_MESH_CONFIG_TYPE_PARAMETER_SIZE;
	timerConfig = (TimerConfig *)configPos;
	configPos += configHeader.timerCount * RF_MESH_CONFIG_TYPE_TIMER_SIZE;
}

void RfMeshLeaf::fetchConfig() {
	validConfig = false;
	appMessage amReq;
	amReq.source = node->addr;
	amReq.dest = nodeConfig.rootAddress;
	amReq.pid = 0;
	amReq.ptype = RF_MESH_PTYPE_CONFIG_REQUEST;
	amReq.len = 2;
	amReq.data[0] = RF_MESH_CONFIG_TYPE_HEADER | RF_MESH_CONFIG_FETCHALL;
	amReq.data[1] = 0;
	node->Send(amReq);
}

uint16_t RfMeshLeaf::getConfigSize() {
	uint16_t res = 0;
	res += configHeader.ioCount * RF_MESH_CONFIG_TYPE_IO_SIZE;
	res += configHeader.networkCount * RF_MESH_CONFIG_TYPE_NETWORK_SIZE;
	res += configHeader.controlCount * RF_MESH_CONFIG_TYPE_CONTROL_SIZE;
	res += configHeader.parameterCount * RF_MESH_CONFIG_TYPE_PARAMETER_SIZE;
	res += configHeader.timerCount * RF_MESH_CONFIG_TYPE_TIMER_SIZE;
	return res;
}

void RfMeshLeaf::saveConfig() {
	eeprom_write_block(&configHeader, CONFIG_ADDR, sizeof configHeader);
	uint8_t* configPos = CONFIG_ADDR + sizeof configHeader;
	uint16_t cs = getConfigSize();
	eeprom_write_block(&configData, configPos, cs);
	nodeConfig.configCrc = calcCRC(&configData, cs);
	nodeConfig.crc = calcCRC(&nodeConfig, sizeof nodeConfig - 2);
	eeprom_write_block(&nodeConfig, NODE_CONFIG_ADDR, sizeof nodeConfig);
	setupIO();
	validConfig = true;
}

void RfMeshLeaf::setupIO() {
	uint8_t capSenseCount = 0;
	for (uint8_t i = 0; i < configHeader.ioCount; i++)
		if (ioConfig[i].ioPrototype == PROTO_IO_D_IN_CAPSENSE)
			capSenseCount++;
	// ToDo : delete if not null
	capSense = new CapacitiveSensor*[capSenseCount];
	uint8_t capSenseIndex = 0;
	for (uint8_t i = 0; i < configHeader.ioCount; i++) {
		uint8_t proto = ioConfig[i].ioPrototype;
		if (proto > 0) {
			if (proto == PROTO_IO_D_IN_CAPSENSE) {
				uint8_t cpinm = (ioConfig[i].pinNumber & 0xF0) >> 4;
				uint8_t cpin = ioConfig[i].pinNumber & 0x0F;
				capSense[capSenseIndex] = new CapacitiveSensor(cpinm, cpin);
				capSenseIndex++;
//			} else if (proto == PROTO_IO_D_IN_CAPSENSE_MAIN) {
//				// do nothing
			} else if (proto == PROTO_IO_D_IN_DS18B20) {
				pinMode(ioConfig[i].pinNumber, INPUT); // this is nothing special - just treat the same as other inputs
				setValue(ioConfig[i].memAddress, 0xFFFF, 0);
			} else if ((proto & 0x80) > 0) { // input
				if ((proto & 0x40) > 0) { // digital
					if ((proto & 0x20) > 0) { // internal pull-up
						pinMode(ioConfig[i].pinNumber, INPUT_PULLUP);
					} else { // no internal pull-up
						pinMode(ioConfig[i].pinNumber, INPUT);
					}
					setValue(ioConfig[i].memAddress, digitalRead(ioConfig[i].pinNumber), 0);
				} else { // analogue
					pinMode(ioConfig[i].pinNumber, INPUT);
//					setValue(ioConfig[i].memAddress, analogRead(ioConfig[i].pinNumber), 0);
				}
			} else { // output
				if ((proto & 0x40) > 0) // digital output
					digitalWrite(ioConfig[i].pinNumber, (proto & 0x20) >> 5);
				pinMode(ioConfig[i].pinNumber, OUTPUT);
			}
		}
	}
}

void RfMeshLeaf::loadConfig() {
	eeprom_read_block(&configHeader, CONFIG_ADDR, sizeof configHeader);
	if (calcCRC(&configHeader, sizeof configHeader) != 0) {
		fetchConfig();
	} else {
		organizeConfigMem();
		uint8_t* configPos = CONFIG_ADDR + sizeof configHeader;
		uint16_t cs = getConfigSize();
		eeprom_read_block(&configData, configPos, cs);
		if (nodeConfig.configCrc != calcCRC(&configData, cs)) {
			fetchConfig();
		} else {
			validConfig = true;
			setupIO();
		}
	}
}

uint8_t RfMeshLeaf::onewire_reset(uint8_t pin) {
	cli();
	digitalWrite(pin, LOW);
	pinMode(pin, OUTPUT);
	delayMicroseconds(480);
	pinMode(pin, INPUT);
	delayMicroseconds(70);
	uint8_t i = digitalRead(pin); // 0=OK, 1=WRONG
	delayMicroseconds(410);
	sei();
	return i;
}

void RfMeshLeaf::onewire_writebit(uint8_t pin, uint8_t data) {
	cli();
	digitalWrite(pin, LOW);
	pinMode(pin, OUTPUT);
	delayMicroseconds((data & 0x01) ? 10 : 65);
	digitalWrite(pin, HIGH);
	sei();
	delayMicroseconds((data & 0x01) ? 55 : 5);
}

void RfMeshLeaf::onewire_writebyte(uint8_t pin, uint8_t data) {
	for (uint8_t bitmask = 0x01; bitmask; bitmask <<= 1)
		onewire_writebit(pin, (data & bitmask) ? 1 : 0);
	pinMode(pin, INPUT);
	digitalWrite(pin, LOW);
}

uint8_t RfMeshLeaf::onewire_readbit(uint8_t pin) {
	cli();
	pinMode(pin, OUTPUT);
	digitalWrite(pin, LOW);
	delayMicroseconds(3);
	pinMode(pin, INPUT);
	delayMicroseconds(10);
	uint8_t data = digitalRead(pin);
	sei();
	delayMicroseconds(45);
	return data;
}

uint8_t RfMeshLeaf::onewire_readbyte(uint8_t pin) {
    uint8_t data = 0;
    for (uint8_t bitmask = 0x01; bitmask; bitmask <<= 1)
		if (onewire_readbit(pin))
			data |= bitmask;
    return data;
}

void RfMeshLeaf::checkInputs() {
	unsigned long timeout;
	uint8_t capSenseIndex = 0;
	uint8_t capsense_do = 0;
	if (capsense_next < millis()) {
		capsense_do = 1;
		capsense_next = millis() + 100;
	}
	uint8_t onewire_do = 0;
	if ((onewire_next & 0x3F) == node->tsLast.second) {
		if ((onewire_next & 0x80) == 0) {
			onewire_do = 0x01; // onewire do read
			if (onewire_next + 5 > 59)
				onewire_next = onewire_next + 5 - 60;
			else
				onewire_next = onewire_next + 5;
			onewire_next |= 0x80;
		} else {
			onewire_do = 0x02; // onewire do fetch
			onewire_next &= 0x7F;
			if (onewire_next + 10 > 59)
				onewire_next = onewire_next + 10 - 60;
			else
				onewire_next = onewire_next + 10;
		}
	}
	for (uint8_t i = 0; i < configHeader.ioCount; i++) {
		uint8_t proto = ioConfig[i].ioPrototype;
		if (proto != PROTO_IO_NONE) {
			if ((proto & 0x80) > 0) { // input
				uint8_t newValue;
				if ((proto & 0x40) > 0) { // digital
					proto &= 0xDF; // don't care about internal pull-up 
					switch (proto) {
						case PROTO_IO_D_IN_BUTTON_ACTIVE_LOW:
							newValue = digitalRead(ioConfig[i].pinNumber);
							setValue(ioConfig[i].memAddress, newValue, (newValue == LOW));
							break;
						case PROTO_IO_D_IN_BUTTON_ACTIVE_HIGH:
							newValue = digitalRead(ioConfig[i].pinNumber);
							setValue(ioConfig[i].memAddress, newValue, (newValue == HIGH));
							break;
						case PROTO_IO_D_IN_SWITCH:
							newValue = digitalRead(ioConfig[i].pinNumber);
							setValue(ioConfig[i].memAddress, newValue, 1);
							break;
						case PROTO_IO_D_IN_ZEROCROSS_ACTIVE_LOW:
						case PROTO_IO_D_IN_ZEROCROSS_ACTIVE_LOW_PU:
							newValue = 0;
							timeout = millis() + 50; // minimum 20ms for 50Hz (or 10ms for half sinus)
							while (millis() < timeout)
								if (digitalRead(ioConfig[i].pinNumber) == LOW)
								{
									newValue = 1;
									break;
								}
							setValue(ioConfig[i].memAddress, newValue, 1);
							break;
						case PROTO_IO_D_IN_ZEROCROSS_ACTIVE_HIGH:
						case PROTO_IO_D_IN_ZEROCROSS_ACTIVE_HIGH_PU:
							newValue = 0;
							timeout = millis() + 50; // minimum 20ms for 50Hz (or 10ms for half sinus)
							while (millis() < timeout)
								if (digitalRead(ioConfig[i].pinNumber) == HIGH)
								{
									newValue = 1;
									break;
								}
							setValue(ioConfig[i].memAddress, newValue, 1);
							break;
						case PROTO_IO_D_IN_CAPSENSE:
							if (capsense_do == 1) {
								long capval = capSense[capSenseIndex]->capacitiveSensor(30);
								newValue = (capval > 0x40);
								setValue(ioConfig[i].memAddress, newValue, 1);
//								setValue(32, (uint16_t)capval, 0);
								capSenseIndex++;
//							if (ioConfig[i].pinNumber < 8) { // only support port D for now
//								uint8_t mask = 1 << ioConfig[i].pinNumber;
//								int total = 0;
//								for (uint8_t k = 0; k < 10; k++) { // measure 10 times
//									// Discharge the pin first by setting it low and output 
//									PORTD &= ~(mask);
//									DDRD |= mask;
//									delay(1);
//									// Make the pin an input WITHOUT the internal pull-up on 
//									DDRD &= ~(mask); 
//									// Now see how long the pin to get pulled up 
//									cli();
//									for(int i = 0; i < 16000; i++) {
//										if (PIND & mask) { 
//											total += i;
//											break; 
//										}
//									} 
//									sei();
									// Discharge the pin again by setting it low and output 
									// It's important to leave the pins low if you want to 
									// be able to touch more than 1 sensor at a time - if 
									// the sensor is left pulled high, when you touch 
									// two sensors, your body will transfer the charge between 
									// sensors. 
//									PORTD &= ~(mask); 
//									DDRD |= mask; 
//								}
//								newValue = (total > 10000);
//								setValue(ioConfig[i].memAddress, newValue, 1);
//							}
							}
							break;
						case PROTO_IO_D_IN_DS18B20:
							if (onewire_do == 0x01) {
								// start reading
								onewire_reset(ioConfig[i].pinNumber);
								onewire_writebyte(ioConfig[i].pinNumber, 0xCC);
								onewire_writebyte(ioConfig[i].pinNumber, 0x44);
							} else if (onewire_do == 0x02) {
								// fetch result
								onewire_reset(ioConfig[i].pinNumber);
								onewire_writebyte(ioConfig[i].pinNumber, 0xCC);
								onewire_writebyte(ioConfig[i].pinNumber, 0xBE);									
								uint16_t val = onewire_readbyte(ioConfig[i].pinNumber);
								val += (onewire_readbyte(ioConfig[i].pinNumber) << 8);
								val <<= 1;
								uint16_t oldval = getValue(ioConfig[i].memAddress);
								if (oldval == 0xFFFF) {
									setValue(ioConfig[i].memAddress, val, 1);
								} else {
									uint16_t valp = (val & 0x8000) ? (val ^ 0xFFFF) + 1 : val;
									uint16_t oldvalp = (oldval & 0x8000) ? (oldval ^ 0xFFFF) + 1 : oldval;
									if ((val & 0x8000) != (oldval & 0x8000)) {
										if (valp + oldvalp >= 2) {
											uint16_t newval = 0;
											if (valp > oldvalp) {
												newval = valp - oldvalp;
												newval >>= 1;
												if (val & 0x8000) {
													newval ^= 0xFFFF;
													newval++;
												}
											} else if (oldvalp > valp) {
												newval = oldvalp - valp;
												newval >>= 1;
												if (oldval & 0x8000) {
													newval ^= 0xFFFF;
													newval++;
												}
											}
											setValue(ioConfig[i].memAddress, newval, 1);
										}
									} else {
										if ((valp >= oldvalp + 2) || (oldvalp >= valp + 2)) {
											uint16_t newval = valp + oldvalp;
											newval >>= 1;
											if (val & 0x8000) {
												newval ^= 0xFFFF;
												newval++;
											}
											setValue(ioConfig[i].memAddress, newval, 1);
										}
									}
								}
								onewire_reset(ioConfig[i].pinNumber);
							}
							break;
					}
				} else { // analogue
//					newValue = analogRead(ioConfig[i].pinNumber);
//					switch (proto) {
//					}
				}
			}
		}
	}
}

void RfMeshLeaf::checkTimer() {
	if (node->tsValid) {
		node->refreshTime();
		if (delayedExecTimer != 255)
			if (delayedExecMillis < millis()) {
				execTimer(delayedExecTimer);
				delayedExecTimer = 255;
			}
		if (lastTimerExecMin == node->tsLast.minute)
			return;
		lastTimerExecMin = node->tsLast.minute;
		for (uint8_t i = 0; i < configHeader.timerCount; i++) {
			uint8_t proto = timerConfig[i].timerPrototype;
			if (proto != PROTO_TIMER_NONE) {
				bool exec = false;
				if ((timerConfig[i].hour & 0x1F) == node->tsLast.hour) {
					if ((timerConfig[i].minute & 0x3F) == node->tsLast.minute) {
						if (isVacation()) {
							if ((timerConfig[i].minute & 0x80) > 0)
								exec = true;
						} else if (isBankHoliday()) {
							if ((timerConfig[i].minute & 0x40) > 0)
								exec = true;
						} else {
							if (timerConfig[i].weekdays & (1 << node->tsLast.weekday) > 0)
								exec = true;
						}
					}
				}
				if (exec) {
					if ((timerConfig[i].hour & 0xE0) > 0) {
						if (delayedExecTimer != 255)
							execTimer(delayedExecTimer);
						delayedExecMillis = millis() + rnd(((timerConfig[i].hour & 0xE0) >> 5) * 5000);
						delayedExecTimer = i;
					} else {
						execTimer(i);
					}
				}
			}
		}
	}
}

void RfMeshLeaf::execTimer(uint8_t timer) {
	TimerConfig tc = timerConfig[timer];
	ParameterConfig * pc = &(parameterConfig[tc.paramStart]);
	uint8_t proto = tc.timerPrototype;
	if (proto != PROTO_TIMER_NONE) {
		switch (proto) {
			case PROTO_TIMER_SETVALUE:
				for (uint16_t i = 0; i < tc.paramCount; i++)
					setValue(pc[i].type, pc[i].value, 1);
				break;
		}
	}	
}

void RfMeshLeaf::rxNetwork() {
	while (node->HasData()) {
		appMessage am = node->Receive();
		switch (am.ptype) {
			case RF_MESH_PTYPE_ANNOUNCE_RESPONSE:
				if (1 == 1) {
					AnnounceResponse *announceResponse = (AnnounceResponse *)am.data;
					if ((nodeConfig.fwVersion != announceResponse->fwVersion) || (nodeConfig.fwCrc != announceResponse->fwCrc))
						reboot();
					if (nodeConfig.configCrc != announceResponse->configCrc)
						fetchConfig();
				}
				continue;
			case RF_MESH_PTYPE_CONFIG_RESPONSE:
				rxConfig(am);
				continue;
			case RF_MESH_PTYPE_DATASYNC:
				for (uint8_t i = 0; i < configHeader.networkCount; i++) {
					uint8_t proto = networkConfig[i].networkPrototype;
					if (proto != PROTO_NETWORK_NONE)
						if ((proto & 0x80) > 0) // rx
							if ((am.source == networkConfig[i].address) || (networkConfig[i].address == RF_MESH_BROADCAST))
								if ((am.dest == node->addr) || ((proto & 0x40) > 0))
									if (am.data[0] == networkConfig[i].sourceMemAddress)
										if (am.data[1] == networkConfig[i].destMemAddress) {
											if (networkConfig[i].destMemAddress <= MEM_BYTE_MAX)
												setValue(networkConfig[i].destMemAddress, am.data[2], 1);
											else
												setValue(networkConfig[i].destMemAddress, am.data[2] + (am.data[3] >> 8), 1);
										}
				}
				continue;
			case RF_MESH_PTYPE_DATA_SET:
				setValue(am.data[0], am.data[1] + (am.data[2] >> 8), 2);
				continue;
			case RF_MESH_PTYPE_DATA_GET_REQUEST:
				if (1 == 1) {
					appMessage amr;
					amr.source = node->addr;
					amr.dest = am.source;
					amr.pid = 0;
					amr.ptype = RF_MESH_PTYPE_DATA_GET_RESPONSE;
					amr.len = 3;
					amr.data[0] = am.data[0];
					uint16_t val = getValue(am.data[0]);
					amr.data[1] = (uint8_t)(val & 0x00FF);
					amr.data[2] = (uint8_t)((val & 0xFF00) >> 8);
					node->Send(amr);
				}
				continue;
			case RF_MESH_PTYPE_CONFIG_REQUEST:
				if (1 == 1) {
					appMessage amr;
					amr.source = node->addr;
					amr.dest = am.source;
					amr.pid = 0;
					amr.ptype = RF_MESH_PTYPE_CONFIG_RESPONSE;
					uint8_t* datasrc;
					uint8_t datalen;
					switch (am.data[0]) {
						case RF_MESH_CONFIG_TYPE_HEADER:
							datasrc = (uint8_t*)(&configHeader);
							datalen = 6;
							break;
						case RF_MESH_CONFIG_TYPE_IO:
							datasrc = (uint8_t*)(&(ioConfig[am.data[1]]));
							datalen = 3;
							break;
						case RF_MESH_CONFIG_TYPE_NETWORK:
							datasrc = (uint8_t*)(&(networkConfig[am.data[1]]));
							datalen = 5;
							break;
						case RF_MESH_CONFIG_TYPE_CONTROL:
							datasrc = (uint8_t*)(&(controlConfig[am.data[1]]));
							datalen = 5;
							break;
						case RF_MESH_CONFIG_TYPE_PARAMETER:
							datasrc = (uint8_t*)(&(parameterConfig[am.data[1]]));
							datalen = 2;
							break;
						case RF_MESH_CONFIG_TYPE_TIMER:
							datasrc = (uint8_t*)(&(timerConfig[am.data[1]]));
							datalen = 6;
							break;
					}
					amr.len = 2 + datalen;
					amr.data[0] = am.data[0];
					amr.data[1] = am.data[1];
					for (uint8_t i = 0; i < datalen; i++)
						amr.data[2 + i] = datasrc[i];
					node->Send(amr);
				}
				continue;
		}
	}
}

void RfMeshLeaf::execControls() {
	for (uint8_t i = 0; i < configHeader.controlCount; i++) {
		uint16_t proto = controlConfig[i].controlPrototype;
		if (proto != PROTO_CONTROL_NONE) {
			switch (proto) {
				case PROTO_CONTROL_ONOFF:
					execControl_OnOff(i);
					break;
				case PROTO_CONTROL_HEATING:
					execControl_Heating(i);
					break;
				case PROTO_CONTROL_BLIND:
					execControl_Blind(i);
					break;
			}		
		}
	}
}

uint8_t RfMeshLeaf::getParam(uint8_t type, ParameterConfig * pc, uint8_t count) {
	for (uint8_t j = 0; j < count; j++)
		if (pc[j].type == type)
			return pc[j].value;
	return 255;
}

void RfMeshLeaf::execControl_OnOff(uint8_t ci) {
	ControlConfig cc = controlConfig[ci];
	ParameterConfig * pc = &(parameterConfig[cc.paramStart]);
	// get current status
	uint8_t oldValue = 0;
	for (uint8_t j = 0; j < cc.paramCount; j++) {
		if (pc[j].type == PARAM_CONTROL_ONOFF_MO) {
			oldValue = getValue(pc[j].value);
			break;
		} else if (pc[j].type == PARAM_CONTROL_ONOFF_MO_INV) {
			oldValue = getValue(pc[j].value) ^ 0x01;
			break;
		}
	}
	uint8_t newValue = oldValue;
	// check if button pressed
	for (uint8_t j = 0; j < cc.paramCount; j++)
		if (pc[j].type == PARAM_CONTROL_ONOFF_MI_TOGGLE)
			if (isFlagged(pc[j].value)) {
				newValue ^= 0x01;
				break;
			}
		else if (pc[j].type == PARAM_CONTROL_ONOFF_MI_TOGGLE_RISING)
			if (isFlagged(pc[j].value) && (getValue(pc[j].value) > 0)) {
				newValue ^= 0x01;
				break;
			}
		else if (pc[j].type == PARAM_CONTROL_ONOFF_MI_TOGGLE_FALLING)
			if (isFlagged(pc[j].value) && (getValue(pc[j].value) == 0)) {
				newValue ^= 0x01;
				break;
			}
	// check if new status set
	for (uint8_t j = 0; j < cc.paramCount; j++)
		if (pc[j].type == PARAM_CONTROL_ONOFF_MI_SET)
			if (isFlagged(pc[j].value))
				newValue = getValue(pc[j].value);
	// set output
	if (newValue != oldValue)
		for (uint8_t j = 0; j < cc.paramCount; j++)
			if (pc[j].type == PARAM_CONTROL_ONOFF_MO)
				setValue(pc[j].value, newValue, 1);
			else if (pc[j].type == PARAM_CONTROL_ONOFF_MO_INV)
				setValue(pc[j].value, newValue ^ 0x01, 1);
}

void RfMeshLeaf::execControl_Heating(uint8_t ci) {
	ControlConfig cc = controlConfig[ci];
	ParameterConfig * pc = &(parameterConfig[cc.paramStart]);
	// get current status
	uint8_t oldValue = getValue(getParam(PARAM_CONTROL_HEATING_MEM_STATUS, pc, cc.paramCount));
	uint8_t newValue = oldValue;
	// check if next button clicked
	for (uint8_t j = 0; j < cc.paramCount; j++)
		if (pc[j].type == PARAM_CONTROL_HEATING_NEXT)
			if (isFlagged(pc[j].value)) {
				newValue++;
				if ((newValue & 0x03) == 3)
					newValue &= 0xFC;
				break;
			}
	// check if new status set
	for (uint8_t j = 0; j < cc.paramCount; j++)
		if (pc[j].type == PARAM_CONTROL_HEATING_SET)
			if (isFlagged(pc[j].value))
				newValue = getValue(pc[j].value);
	// check if window open
	uint8_t window = 0x00;
	for (uint8_t j = 0; j < cc.paramCount; j++)
		if (pc[j].type == PARAM_CONTROL_HEATING_WINDOW)
			if (getValue(pc[j].value) == 1) {
				window = 0x80;
				break;
			}
	if (window == 0)
		newValue &= 0x7F;
	else
		newValue |= 0x80;
	// get target temperature based on status
	uint8_t targetTemp;
	if (newValue & 0x80 == 0)
		targetTemp = getParam(PARAM_CONTROL_HEATING_VAL_FROST + newValue, pc, cc.paramCount);
	else
		targetTemp = getParam(PARAM_CONTROL_HEATING_VAL_FROST, pc, cc.paramCount);
	// get current temperature based on sensor input and offset
	uint8_t currentTemp = getValue(getParam(PARAM_CONTROL_HEATING_SENSOR, pc, cc.paramCount));
	currentTemp += (getParam(PARAM_CONTROL_HEATING_VAL_OFFSET, pc, cc.paramCount) - 100);
	// get valve state and define new valve state based on temperature
	uint8_t valve = getValue(getParam(PARAM_CONTROL_HEATING_VALVE, pc, cc.paramCount));
	if ((valve == 0) && (currentTemp < targetTemp - 5))
		valve = 1;
	else if ((valve == 1) && (currentTemp > targetTemp + 5))
		valve = 0;
	// set outputs (valve and LED)
	for (uint8_t j = 0; j < cc.paramCount; j++) {
		if (pc[j].type == PARAM_CONTROL_HEATING_VALVE) {
			setValue(pc[j].value, valve, 1);
		} else if (pc[j].type == PARAM_CONTROL_HEATING_IS_FROST) {
			setValue(pc[j].value, ((newValue & 0x03) == 0), 1);
		} else if (pc[j].type == PARAM_CONTROL_HEATING_IS_NIGHT) {
			setValue(pc[j].value, ((newValue & 0x03) == 1), 1);
		} else if (pc[j].type == PARAM_CONTROL_HEATING_IS_DAY) {
			setValue(pc[j].value, ((newValue & 0x03) == 2), 1);
		}
	}
	// save new status
	setValue(getParam(PARAM_CONTROL_HEATING_MEM_STATUS, pc, cc.paramCount), newValue, 1);
}

void RfMeshLeaf::execControl_Blind(uint8_t ci) {
	ControlConfig cc = controlConfig[ci];
	ParameterConfig * pc = &(parameterConfig[cc.paramStart]);
	// get current status
	uint8_t oldValue = getValue(getParam(PARAM_CONTROL_BLIND_MO_DIRECTION, pc, cc.paramCount));
	oldValue += 2 * getValue(getParam(PARAM_CONTROL_BLIND_MO_ENGINE, pc, cc.paramCount));
	uint8_t newValue = oldValue;
	// if engine running then check timer / update timer / stop motor if done
	uint8_t timerAddr = getParam(PARAM_CONTROL_BLIND_MEM_TIMER, pc, cc.paramCount);
	if ((oldValue & 0x02) > 0) {
		uint8_t timer = getValue(timerAddr);
		if (node->tsLast.second == (timer & 0x3F)) {
			if ((timer & 0xC0) == 0)
				newValue &= 0xFD; // engine off
			else
				setValue(timerAddr, timer - 0x40 - 0x01, 0); // one minute down (and one second down to avoid double counting)
		}
	}
	// if auto button pressed then toggle auto state (stored in output)
	uint8_t autoVal = getValue(getParam(PARAM_CONTROL_BLIND_MO_AUTO, pc, cc.paramCount));
	for (uint8_t j = 0; j < cc.paramCount; j++)
		if (pc[j].type == PARAM_CONTROL_BLIND_MI_AUTO)
			if (isFlagged(pc[j].value)) {
				autoVal ^= 0x01;
				break;
			}
	setValue(PARAM_CONTROL_BLIND_MO_AUTO, autoVal, 1);
	// get window state (1 means open)
	uint8_t windowVal = 0;
	for (uint8_t j = 0; j < cc.paramCount; j++)
		if (pc[j].type == PARAM_CONTROL_BLIND_MI_WINDOW)
			if (getValue(pc[j].value) == 1) {
				windowVal = 1;
				break;
			}
	// if auto is on and window is not open then check for timer input (if not then just skip)
	if ((autoVal == 1) && (windowVal == 0)) {
		// ToDo : 
		//   check for sun detector only in here (continuous sun for more than x minutes)
		//     means: if position higher than sun position then go further down
		//            if position lower than sun position then stay
		//   check for timer inputs only in here
		for (uint8_t j = 0; j < cc.paramCount; j++) {
			if (pc[j].type == PARAM_CONTROL_BLIND_MI_SET_AUTO) {
				if (isFlagged(pc[j].value)) {
					uint8_t autoDir = getValue(pc[j].value);
					if ((oldValue & 0x03) == 0x03 - autoDir) { // engine moving in wrong direction
						setValue(PARAM_CONTROL_BLIND_MO_ENGINE, 0, 1); // stop engine immediately
						delay(10); // wait 10 ms
					}
					newValue &= 0xFC;
					newValue |= (0x02 + autoDir);
					setValue(timerAddr, 0x80 + node->tsLast.second, 0); // two minute runtime
					break;
				}
			} else if (pc[j].type == PARAM_CONTROL_BLIND_MI_SUN) {
				if (isFlagged(pc[j].value)) {
					uint8_t autoDir = getValue(pc[j].value);
					if ((oldValue & 0x03) == 0x03 - autoDir) { // engine moving in wrong direction
						setValue(PARAM_CONTROL_BLIND_MO_ENGINE, 0, 1); // stop engine immediately
						delay(10); // wait 10 ms
					}
					newValue &= 0xFC;
					newValue |= (0x02 + autoDir);
					if (autoDir == 0) {
						// calculate runtime based on sun position
						uint8_t sunPos = getParam(PARAM_CONTROL_BLIND_SUN_POSITION, pc, cc.paramCount);
						uint8_t endTime = 0;
						while (sunPos > 60) {
							endTime += 0x40;
							sunPos -= 60;
						}
						if (sunPos + node->tsLast.second > 60) {
							endTime += 0x40;
							endTime += (sunPos + node->tsLast.second - 60);
						} else {
							endTime += (sunPos + node->tsLast.second);
						}
						setValue(timerAddr, endTime, 0);
					} else {
						setValue(timerAddr, 0x80 + node->tsLast.second, 0); // two minute runtime up
					}
					break;
				}
			}
		}
	}
	// react on up/down buttons
	for (uint8_t j = 0; j < cc.paramCount; j++) {
		if (pc[j].type == PARAM_CONTROL_BLIND_MI_UP)
			if (isFlagged(pc[j].value)) {
				if ((newValue & 0x02) == 0) {
					newValue |= 0x01; // set direction to up
					newValue |= 0x02; // start engine
					setValue(timerAddr, 0x80 + node->tsLast.second, 0); // one minute down
				} else if ((newValue & 0x01) > 0)
					newValue &= 0xFD; // stop engine
				break;
			} else if (pc[j].type == PARAM_CONTROL_BLIND_MI_DOWN)
			if (isFlagged(pc[j].value)) {
				if ((newValue & 0x02) == 0) {
					newValue &= 0xFE; // set direction to down
					newValue |= 0x02; // start engine
					setValue(timerAddr, 0x80 + node->tsLast.second, 0); // one minute down
				} else if ((newValue & 0x01) > 0)
					newValue &= 0xFD; // stop engine
				break;
			}
	}
	// set outputs
	setValue(getParam(PARAM_CONTROL_BLIND_MO_DIRECTION, pc, cc.paramCount), newValue & 0x01, 1);
	setValue(getParam(PARAM_CONTROL_BLIND_MO_ENGINE, pc, cc.paramCount), (newValue & 0x02) >> 1, 1);
}

void RfMeshLeaf::txNetwork() {
	for (uint8_t i = 0; i < configHeader.networkCount; i++) {
		uint8_t proto = networkConfig[i].networkPrototype;
		if (proto != PROTO_NETWORK_NONE) {
			if ((proto & 0x80) == 0) { // tx
				switch (proto) {
					case PROTO_NETWORK_TX:
						if (isFlagged(networkConfig[i].sourceMemAddress)) {
							appMessage am;
							am.source = node->addr;
							am.dest = networkConfig[i].address;
							am.pid = 0;
							am.ptype = RF_MESH_PTYPE_DATASYNC;
							am.len = 4;
							am.data[0] = networkConfig[i].sourceMemAddress;
							am.data[1] = networkConfig[i].destMemAddress;
							uint16_t val = getValue(networkConfig[i].sourceMemAddress);
							am.data[2] = (uint8_t)(val & 0x00FF);
							am.data[3] = (uint8_t)((val & 0xFF00) >> 8);
							node->Send(am);
						}
						break;
				}
			}
		}
	}
}

void RfMeshLeaf::setOutputs() {
	for (uint8_t i = 0; i < configHeader.ioCount; i++) {
		uint8_t proto = ioConfig[i].ioPrototype;
		if (proto != PROTO_IO_NONE) {
			if ((proto & 0x80) == 0) { // output
				uint8_t newValue = getValue(ioConfig[i].memAddress);
				if ((proto & 0x40) > 0) { // digital
					// ToDo : confirm: same for all digital outputs...
//					switch (proto) {
//						case PROTO_IO_D_OUT_LED:
							digitalWrite(ioConfig[i].pinNumber, newValue);
//							break;
//					}
				} else { // analogue
//					newValue = analogRead(ioConfig[i].pinNumber);
//					switch (proto) {
//					}
				}
			}
		}
	}
}

/*
void RfMeshLeaf::sendDebug(uint8_t position, uint8_t data1, uint8_t data2) {
	meshMessage mm;
	mm.tc = 255;
	mm.to = 0x0001;
	mm.from = node->addr;
	mm.source = node->addr;
	mm.dest = 0x0001;
	mm.pid = (node->nextpid)++;
	mm.ptype = RF_MESH_PTYPE_DEBUG;
	mm.cost = 1;
	mm.len = 3;
	mm.data[0] = position;
	mm.data[1] = data1;
	mm.data[2] = data2;
	node->SendWaitHopAck(mm);
}
*/