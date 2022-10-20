#pragma once

#include "MAX31855.h"
#include "thermocouple.h"

class Max31855Thermocouple : public Thermocouple {
 public:
  Max31855Thermocouple(int cs);

  bool Begin();

  float ReadCelsius() override;

  uint8_t GetReadError() const override;

 private:
  MAX31855_Class sensor_;

  const int cs_;
};