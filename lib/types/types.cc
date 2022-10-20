#include "types.h"

uint16_t analog_read_value = 0;
uint16_t analogRead(int pin) { return analog_read_value; }
void SetAnalogRead(int pin, uint16_t value) { analog_read_value = value; }

void analogReadResolution(int resolution) {}