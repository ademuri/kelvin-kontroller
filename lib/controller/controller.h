#pragma once

#include <arduino-timer.h>
#include <median-filter.h>

#include "types.h"

class Controller {
 public:
  Controller();
  virtual ~Controller();

  // Runs one iteration of the control loop
  void Step();

  // Actuators
  virtual void SetFan(const uint8_t pwm) = 0;
  virtual void SetHeater(const uint8_t pwm) = 0;
  virtual void SetStir(const bool on) = 0;

  // Returns the current actual setpoint for the heater
  uint8_t GetFanValue() { return fan_value_; }

  // Returns the current actual setpoint for the heater
  uint8_t GetHeaterValue() { return heater_value_; }

  // Returns the value of the relay, which controls power to the SSR
  bool GetRelayValue() { return relay_value_; }

  // Sensors
  // Reads the thermocouple for the bean mass
  virtual float ReadBeanTempF() = 0;

  // Reads the thermocouple for the roaster environment (heater output)
  virtual float ReadEnvTempF() = 0;

  // Reads the thermistor on the control board.
  virtual float ReadAmbientTempF() = 0;

 protected:
  uint8_t fan_target_ = 0;
  uint8_t fan_value_ = 0;
  uint8_t heater_target_ = 0;
  uint8_t heater_value_ = 0;
  bool stir_value_ = 0;

  bool relay_value_ = 1;

  uint8_t fan_min_ = 0;
  uint8_t fan_max_ = 255;
  uint8_t heater_min_ = 0;
  uint8_t heater_max_ = 255;

  MedianFilter<float, float, 5> bean_temp_filter_ =
      MedianFilter<float, float, 5>([this]() { return ReadBeanTempF(); });
  MedianFilter<float, float, 5> env_temp_filter_ =
      MedianFilter<float, float, 5>([this]() { return ReadEnvTempF(); });
  MedianFilter<float, float, 5> ambient_temp_filter_ =
      MedianFilter<float, float, 5>([this]() { return ReadAmbientTempF(); });
};
