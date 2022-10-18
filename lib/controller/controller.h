#pragma once

#include <types.h>

class Controller {
 public:
  Controller() {}

  virtual void SetFan(uint8_t pwm);
  virtual void SetHeater(uint8_t pwm);

 protected:
  uint8_t fan_target_ = 0;
  uint8_t fan_value_ = 0;
  uint8_t heater_target_ = 0;
  uint8_t heater_value_ = 0;

  uint8_t fan_min_ = 0;
  uint8_t fan_max_ = 255;
  uint8_t heater_min_ = 0;
  uint8_t heater_max_ = 255;
};
