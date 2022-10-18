#include <algorithm>

#include "arduino-controller.h"

void ArduinoController::SetFan(uint8_t pwm) {
  fan_target_ = pwm;
  fan_value_ = std::min(fan_max_, pwm);
  fan_value_ = std::max(fan_min_, pwm);

  if (fan_value_ == 0 && kForceDisableFan) {
    digitalWrite(kFanEn, LOW);
  } else {
    digitalWrite(kFanEn, HIGH);
    analogWrite(kFanPwmOut, fan_value_);
  }
}

void ArduinoController::SetHeater(uint8_t pwm) {
  heater_target_ = pwm;
  heater_value_ = std::min(heater_max_, pwm);
  heater_value_ = std::max(heater_min_, pwm);

  analogWrite(kSsrEn, heater_value_);
}