
#include "FirmataClient.h"

void FirmataClientClass::tone(uint8_t pin, int frequency, int duration) {
  // Send data
  firmataStream->write(START_SYSEX);
  firmataStream->write(TONE_DATA);
  firmataStream->write((byte)TONE_PLAY);
  firmataStream->write(pin);
  firmataStream->write(frequency & 0x7F);
  firmataStream->write(frequency >> 7);
  firmataStream->write(duration & 0x7F);
  firmataStream->write(duration >> 7);
  firmataStream->write(END_SYSEX);
}

void FirmataClientClass::noTone(uint8_t pin) {
  // Send data
  firmataStream->write(START_SYSEX);
  firmataStream->write(TONE_DATA);
  firmataStream->write((byte)TONE_STOP);
  firmataStream->write(pin);
  firmataStream->write(END_SYSEX);
}