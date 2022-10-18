#pragma once

#include <types.h>

class Controller {
 public:
  Controller() {}

  virtual void SetFan(const uint8_t pwm);
  virtual void SetHeater(const uint8_t pwm);

  // Returns the current actual setpoint for the heater
  uint8_t GetFanValue() { return fan_value_; }

  // Returns the current actual setpoint for the heater
  uint8_t GetHeaterValue() { return heater_value_; }

  // Returns the value of the relay, which controls power to the SSR
  bool GetRelayValue() { return relay_value_; }

 protected:
  uint8_t fan_target_ = 0;
  uint8_t fan_value_ = 0;
  uint8_t heater_target_ = 0;
  uint8_t heater_value_ = 0;

  bool relay_value_ = 1;

  uint8_t fan_min_ = 0;
  uint8_t fan_max_ = 255;
  uint8_t heater_min_ = 0;
  uint8_t heater_max_ = 255;
};
