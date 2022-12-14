#pragma once

#include <cmath>

#include "types.h"

class Thermistor {
 public:
  Thermistor(int pin, float analog_reference_volts);

  static constexpr uint32_t kDividerOhms = 4700;
  static constexpr int kAnalogReadResolution = 10;
  static constexpr uint32_t kAnalogReadBase = 1 << kAnalogReadResolution;

  // Note: these functions don't modify *this* class, but they do modify global
  // state (analogReadResolution).
  float ReadResistance() const;
  float ReadFahrenheit() const;

  static float ConvertResistanceToKelvin(float resistance);
  static float ConvertResistanceToCelsius(float resistance);
  static float ConvertResistanceToFahrenheit(float resistance);

  static constexpr float kKelvin = 273.15;

  // Visible for testing
  // Resistance at 25C
  static constexpr float resistance_ = 10000;
  static constexpr float b25_50_ = 3380;
  static constexpr float b25_85_ = 3434;
  static constexpr float b25_100_ = 3455;

  // See https://electronics.stackexchange.com/a/51870/242186
  static float ResistanceAtTemp(const float temp, const float beta) {
    return (resistance_) /
           expf(beta * (1 / (kKelvin + 25.0) - 1 / (kKelvin + temp)));
  }

 private:
  const float analog_reference_volts_;
  const int pin_;

  static const float resistance_50c_;
  static const float resistance_85c_;
  static const float resistance_100c_;

  static const float T1_;
  static const float T2_;
  static const float T3_;
  static const float L1_;
  static const float L2_;
  static const float L3_;
  static const float Y1_;
  static const float Y2_;
  static const float Y3_;
  static const float G2_;
  static const float G3_;
  static const float A_;
  static const float B_;
  static const float C_;
};
