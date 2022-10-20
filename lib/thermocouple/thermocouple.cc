#include "thermocouple.h"

float Thermocouple::ReadFahrenheit() { return ReadCelsius() * 1.8 + 32; }
