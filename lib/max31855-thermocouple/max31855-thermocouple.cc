#include "max31855-thermocouple.h"

Max31855Thermocouple::Max31855Thermocouple(int cs) : cs_(cs) {}

bool Max31855Thermocouple::Begin() { return sensor_.begin(cs_); }

float Max31855Thermocouple::ReadCelsius() { return sensor_.readProbe(); }

uint8_t Max31855Thermocouple::GetReadError() const { return sensor_.fault(); }
