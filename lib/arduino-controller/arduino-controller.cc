#include <algorithm>

#include "arduino-controller.h"

void ArduinoController::Init() {
  pinMode(kFanEn, OUTPUT);
  pinMode(kFanPwmOut, OUTPUT);
  pinMode(kFanTach, OUTPUT);
  pinMode(kStirEn, OUTPUT);
  pinMode(kSsrEn, OUTPUT);
  pinMode(kRelayEn, OUTPUT);
  pinMode(kBuzzer, OUTPUT);
  pinMode(kLed, OUTPUT);

  pinMode(kThermistor1, INPUT);
  pinMode(kThermistor2, INPUT);

  bean_temp_filter_.SetMinRunInterval(kFilterRunInterval);
  env_temp_filter_.SetMinRunInterval(kFilterRunInterval);
  ambient_temp_filter_.SetMinRunInterval(kFilterRunInterval);
  
  // TODO: check sensor state and enable the relay
  Controller::Init();
}

void ArduinoController::Step() {
  bean_temp_filter_.Run();
  env_temp_filter_.Run();
  ambient_temp_filter_.Run();
  Controller::Step();
}

void ArduinoController::SetFan(uint8_t pwm) {
  Controller::SetFan(pwm);

  if (fan_value_ == 0 && kForceDisableFan) {
    digitalWrite(kFanEn, LOW);
  } else {
    digitalWrite(kFanEn, HIGH);
    analogWrite(kFanPwmOut, fan_value_);
  }
}

void ArduinoController::SetHeater(uint8_t pwm) {
  Controller::SetHeater(pwm);
  analogWrite(kSsrEn, heater_value_);
}

void ArduinoController::SetStir(bool on) {
  Controller::SetStir(on);
  digitalWrite(kStirEn, on);
}

void ArduinoController::SetRelay(bool on) {
  Controller::SetRelay(on);
  digitalWrite(kRelayEn, on);
}

// TODO: implement these
float ArduinoController::ReadBeanTempF() {
  return bean_temp_filter_.GetFilteredValue();
}

float ArduinoController::ReadEnvTempF() {
  return env_temp_filter_.GetFilteredValue();
}

float ArduinoController::ReadAmbientTempF() {
  return ambient_temp_filter_.GetFilteredValue();
}