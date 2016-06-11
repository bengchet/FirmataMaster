// 
// 
// 

#include "FirmataClient.h"

#define DO_REPORT_ANALOG

static void checkStream(FirmataClientClass* firmata);
#ifdef DEBUG_CAPABILITIES
static String mode2string(int mode);
#endif // DEBUG_CAPABILITIES

FirmataClientClass::FirmataClientClass()
{
}

void FirmataClientClass::begin(Stream &stream)
{
	firmataStream = &stream;

	tk.attach_ms(20, checkStream, this);

	// enable all ports; firmware should ignore non-existent ones
	for (int i = 0; i < MAX_PORTS; i++) {
		firmataStream->write(REPORT_DIGITAL | i);
		firmataStream->write(1);
	}

	delay(1000);
	
	queryCapabilities();
	delay(1000);
#ifdef DO_REPORT_ANALOG
	queryAnalogMapping();
	delay(1000);


	for (int i = 0; i < MAX_ANALOG_PINS; i++) {
		firmataStream->write(REPORT_ANALOG | i);
		firmataStream->write((uint8_t)1);
	}
#endif // DO_REPORT_ANALOG

}

/**
* Returns the last known value read from the digital pin: HIGH or LOW.
*
* @param pin the digital pin whose value should be returned (from 2 to 13,
* since pins 0 and 1 are used for serial communication)
*/
int FirmataClientClass::digitalRead(int pin) {
	int value = (digitalInputData[pin >> 3] >> (pin & 0x07)) & 0x01;
#ifdef DEBUG_DIGITAL
	DBG_PORT.printf("-DigitalRead: pin %d value %d\r\n", pin, value);
#endif // DEBUG_DIGITAL	
	return value;
}

bool FirmataClientClass::getStreamAvailable()
{
	return firmataStream->available();
}

static void checkStream(FirmataClientClass* firmata) {
	FirmataClientClass *firmataInstance = firmata;
	if (!firmataInstance->checkingStream) {

		firmataInstance->checkingStream = true;
		while (firmataInstance->getStreamAvailable()) {
			firmataInstance->handleData();
		}
		firmataInstance->checkingStream = false;
	}
}

/**
* Write to a digital pin (the pin must have been put into output mode with
* pinMode()).
*
* @param pin the pin to write to (from 2 to 13)
* @param value the value to write: Arduino.LOW (0 volts) or Arduino.HIGH
* (5 volts)
*/
void FirmataClientClass::digitalWrite(int pin, int value) {
#ifdef DEBUG_DIGITAL
	DBG_PORT.printf("-Digital Write: pin %d value %d\r\n", pin, value);
#endif // DEBUG_DIGITAL	
	int portNumber = (pin >> 3) & 0x0F;

	if (value == 0)
		digitalOutputData[portNumber] &= ~(1 << (pin & 0x07));
	else
		digitalOutputData[portNumber] |= (1 << (pin & 0x07));

	firmataStream->write(DIGITAL_MESSAGE | portNumber);
	firmataStream->write(digitalOutputData[portNumber] & 0x7F);
	firmataStream->write(digitalOutputData[portNumber] >> 7);
}

void FirmataClientClass::pinMode(int pin, int mode)
{
#ifdef DEBUG_PINS
	DBG_PORT.printf("-Pin mode: pin %d mode %d\r\n", pin, mode);
#endif // DEBUG_PINS
	firmataStream->write(SET_PIN_MODE);
	firmataStream->write(pin);
	firmataStream->write(mode);
}

void FirmataClientClass::setDigitalInputs(int portNumber, int portData) {
#ifdef DEBUG_DIGITAL
	DBG_PORT.printf("-Set digital inputs: port %d data %x\r\n", portNumber, portData);
#endif // DEBUG_DIGITAL
	digitalInputData[portNumber] = portData;
}

void FirmataClientClass::setAnalogInput(int pin, int value) {
#ifdef DEBUG_ANALOG
	//DBG_PORT.printf("-Set analog inputs: pin %d value %d\r\n", pin, value);
#endif // DEBUG_ANALOG	
	analogInputData[pin] = value;
}

void FirmataClientClass::setSamplingInterval(int interval) {
#ifdef DEBUG_PINS
	DBG_PORT.printf("Set sampling interval at %d\r\n", interval);
#endif // DEBUG_PINS
	firmataStream->write(START_SYSEX);
	firmataStream->write(SAMPLING_INTERVAL);
	firmataStream->write(interval & 0xFF);
	firmataStream->write((interval >> 8) & 0xFF);
	firmataStream->write(END_SYSEX);
}

void FirmataClientClass::setVersion(int majorVersion, int minorVersion) {
#ifdef DEBUG_FIRMATA
	DBG_PORT.printf("Firmata Version: %d %d\r\n", majorVersion, minorVersion);
#endif //DEBUG_FIRMATA
	this->majorVersion = majorVersion;
	this->minorVersion = minorVersion;
}

void FirmataClientClass::queryCapabilities() {
#ifdef DEBUG_FIRMATA_PINS
	DBG_PORT.print("Query capabilities\r\n");
#endif // DEBUG_FIRMATA_PINS
	firmataStream->write(START_SYSEX);
	firmataStream->write(CAPABILITY_QUERY);
	firmataStream->write(END_SYSEX);
}

void FirmataClientClass::queryAnalogMapping() {
#ifdef DEBUG_ANALOG
	DBG_PORT.print("Query analog mapping\r\n");
#endif // DEBUG_ANALOG	
	firmataStream->write(START_SYSEX);
	firmataStream->write(ANALOG_MAPPING_QUERY);
	firmataStream->write(END_SYSEX);
}

#ifdef DEBUG_CAPABILITIES
pin * FirmataClientClass::getStoredCapabilities()
{
	if (gotCapabilities) {
		return pins;
	}
	else return NULL;
}

void FirmataClientClass::printCapabilities()
{
	DBG_PORT.println(__PRETTY_FUNCTION__);
	for (int i = 0; i < MAX_PINS; i++) {
		if (pins[i].available){
			DBG_PORT.printf("Pin %d modes\r\n", i);
			for (int j = 0; j < NUMBER_OF_MODES; j++) {
				if (pins[i].capability[j].supported) {
					DBG_PORT.printf("  Mode: %d = %s, resolution: %d", j, mode2string(j).c_str(), pins[i].capability[j].resolution);
					if (j == ANALOG) {
						DBG_PORT.printf(", analog channel: %d", pins[i].analogChannel);
					}
					DBG_PORT.println();
				}
			}
		}
	}
}

static String mode2string(int mode) {
	switch (mode) {
		case INPUT:
			return ("Input");
			break;
		case OUTPUT:
			return ("Output");
			break;
		case ANALOG:
			return ("Analog");
			break;
		case PWM:
			return ("PWM");
			break;
		case SERVO:
			return ("Servo");
			break;
		case SHIFT:
			return ("Shift");
			break;
		case I2C:
			return ("I2C");
			break;
		case ONEWIRE:
			return ("OneWire");
			break;
		case STEPPER:
			return ("Stepper");
			break;
		case ENCODER:
			return ("Encoder");
			break;
		case SERIAL:
			return ("Serial");
			break;
		case PULLUP:
			return ("PULLUP");
			break;
		default:
			return ("Mode not implemented");
	}
}
#endif // DEBUG_CAPABILITIES

void FirmataClientClass::processInput(int inputData) {
	int command;

	if (parsingSysex) {
		if (inputData == END_SYSEX) {
			parsingSysex = false;
			processSysexMessage();
		}
		else {
			storedInputData[sysexBytesRead] = inputData;
			sysexBytesRead++;
		}
	}
	else if (waitForData > 0 && inputData < 128) {
		waitForData--;
		storedInputData[waitForData] = inputData;

		if (executeMultiByteCommand != 0 && waitForData == 0) {
			//we got everything
			switch (executeMultiByteCommand) {
			case DIGITAL_MESSAGE:
#ifdef DEBUG_DIGITAL
				DBG_PORT.println("DIGITAL MESSAGE");
#endif // DEBUG_DIGITAL	
				setDigitalInputs(multiByteChannel, (storedInputData[0] << 7) + storedInputData[1]);
				break;
			case ANALOG_MESSAGE:
#ifdef DEBUG_ANALOG
				DBG_PORT.println("ANALOG MESSAGE");
#endif // DEBUG_ANALOG
				setAnalogInput(multiByteChannel, (storedInputData[0] << 7) + storedInputData[1]);
				break;
			case REPORT_VERSION:
#ifdef DEBUG_FIRMATA
				DBG_PORT.println("REPORT VERSION");
#endif // DEBUG_FIRMATA
				setVersion(storedInputData[1], storedInputData[0]);
				break;
			}
		}
	}
	else {
		if (inputData < 0xF0) {
			command = inputData & 0xF0;
			multiByteChannel = inputData & 0x0F;
		}
		else {
			command = inputData;
			// commands in the 0xF* range don't use channel data
		}
#ifdef DEBUG_FIRMATA_PROTOCOL
		//DBG_PORT.printf("Firmata command: %X %X\r\n", command, multiByteChannel);
#endif // DEBUG_FIRMATA_PROTOCOL
		switch (command) {
			case DIGITAL_MESSAGE:
			case ANALOG_MESSAGE:
			case REPORT_VERSION:
				//Serial1.print("<");
				waitForData = 2;
				executeMultiByteCommand = command;
				break;
			case START_SYSEX:
				parsingSysex = true;
				sysexBytesRead = 0;
				break;
		}
	}
}

void FirmataClientClass::handleData()
{
	int data = -1;
	if (firmataStream->available() > 0) {
		data = firmataStream->read();
#ifdef DEBUG_PROTOCOL_BYTES
		String temp = (data < 0x10 ? " 0x0" : " 0x");
		Serial1.printf("%s%X", temp.c_str(), data);
#endif // DEBUG_PROTOCOL_BYTES	
		if (data != -1) {
			processInput(data);
		}
	}

}

void FirmataClientClass::processSysexMessage() {
	//    System.out.print("[ ");
	//    for (int i = 0; i < storedInputData.length; i++) System.out.print(storedInputData[i] + " ");
	//    System.out.println("]");
#ifdef DEBUG_SYSEX
	DBG_PORT.printf("Process SysEx: SE Command: %X\r\n", storedInputData[0]);
#endif // DEBUG_SYSEX
	switch (storedInputData[0]) { //first byte in buffer is command
      case CAPABILITY_RESPONSE:
#ifdef DEBUG_SYSEX
		  DBG_PORT.print("Capability response\r\n");
#endif // DEBUG_SYSEX
#ifdef DEBUG_CAPABILITIES
		  gotCapabilities = true;
#endif // DEBUG_CAPABILITIES
		  for (int pin = 0; pin < MAX_PINS; pin++) {
          pinModes[pin] = 0;
#ifdef DEBUG_CAPABILITIES
		  pins[pin].available = false;
#endif //DEBUG_CAPABILITIES
        }
        for (int i = 1, pin = 0; pin < MAX_PINS; pin++) {
          for (;;) {
            int val = storedInputData[i++];
            if (val == 127) break;
            pinModes[pin] |= (1 << val);
#ifdef DEBUG_CAPABILITIES
			pins[pin].available = true;
			if (val >= 0 && val < NUMBER_OF_MODES) {
				pins[pin].capability[val].supported = true;
			}
			int res = storedInputData[i++];
			pins[pin].capability[val].resolution = res;
#else
			i++; // skip mode resolution for now
#endif // DEBUG_CAPABILITIES
			//i++; 
          }
          if (i == sysexBytesRead) break;
        }
        for (int port = 0; port < MAX_PINS; port++) {
          boolean used = false;
          for (int pin = 0; pin < 8; pin++) {
            if (pinModes[port * 8 + pin] & (1 << INPUT) != 0) used = true;
          }
          if (used) {
			  firmataStream->write(REPORT_DIGITAL | port);
			  firmataStream->write(1);
          }
        }
        break;
	case ANALOG_MAPPING_RESPONSE:
#ifdef DEBUG_SYSEX
		DBG_PORT.print("Analog mapping response\r\n");
#endif // DEBUG_SYSEX
		for (int pin = 0; pin < MAX_ANALOG_PINS; pin++) {
			analogChannel[pin] = 127;
#ifdef DEBUG_CAPABILITIES
			pins[pin].analogChannel = 127; 
#endif //DEBUG_CAPABILITIES
		}
		for (int i = 1; i < sysexBytesRead; i++) {
			int val = storedInputData[i];
			analogChannel[i - 1] = val;
#ifdef DEBUG_CAPABILITIES
			if (val < 127)
				pins[i-1].analogChannel = val;
#endif // DEBUG_CAPABILITIES
		}
		/*for (int pin = 0; pin < MAX_ANALOG_PINS; pin++) {
			if (analogChannel[pin] != 127) {
				firmataStream->write(REPORT_ANALOG | analogChannel[pin]);
				firmataStream->write(1);
			}
		}*/
		break;
	case REPORT_FIRMWARE:
		firmware = "";
		setVersion(storedInputData[1], storedInputData[2]);

		for (int i = 3; i < sysexBytesRead; i = i+2) {
			firmware.concat((char)storedInputData[i]);
		}
#ifdef DEBUG_SYSEX
		DBG_PORT.printf("SysEx bytes read: %d\r\n", sysexBytesRead);
		DBG_PORT.printf("Firmware: %s\r\n", firmware.c_str());
#endif //DEBUG_SYSEX		
	}
}


FirmataClientClass FirmataClient;

