#pragma once

#include "types.h"

class Thermocouple {
 public:
  virtual float ReadCelsius();
  float ReadFahrenheit();

  virtual uint8_t GetReadError() const;
};