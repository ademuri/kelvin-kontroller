#include "types.h"


#ifndef ARDUINO
uint16_t analog_read_value = 0;
uint16_t analogRead(int pin) { return analog_read_value; }
void SetAnalogRead(int pin, uint16_t value) { analog_read_value = value; }

void analogReadResolution(int resolution) {}

uint32_t millis_ = 0;
uint32_t millis() { return millis_; }
void SetMillis(uint32_t millis) { millis_ = millis; }

#endif