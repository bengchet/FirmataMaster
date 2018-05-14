// 
// 
// 


#include "FirmataClient.h"

#ifdef FIRMATA_PWM_OUTPUT_SUPPORT
void FirmataClientClass::analogWrite(int pin, int value) {
#ifdef DEBUG_PWM_OUTPUT
	DBG_PORT.printf("AnalogWrite pin %d, value %d\r\n", pin, value);
#endif DEBUG_PWM_OUTPUT
	pinMode(pin, PWM);
	firmataStream->write(ANALOG_MESSAGE | (pin & 0x0F));
	firmataStream->write(value & 0x7F);
	firmataStream->write(value >> 7);
}
#endif // FIRMATA_PWM_OUTPUT_SUPPORT