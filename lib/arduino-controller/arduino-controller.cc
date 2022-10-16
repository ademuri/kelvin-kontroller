#include "arduino-controller.h"

void ArduinoController::SetFan(uint8_t pwm) {
  if (pwm == 0 && kForceDisableFan) {
    digitalWrite(kFanEn, LOW);
  } else {
    digitalWrite(kFanEn, HIGH);
    analogWrite(kFanPwmOut, pwm);
  }
}