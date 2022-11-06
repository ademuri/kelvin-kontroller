#include "arduino-controller.h"

#include <algorithm>

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

  analogWriteFrequency(30000);

  bean_therm_.Begin();
  env_therm_.Begin();

  // Filters will return 0 until they've been primed, which will cause a fault.
  for (uint32_t n = 0; n < kFilterSize; n++) {
    bean_temp_filter_.Run();
    env_temp_filter_.Run();
    ambient_temp_filter_.Run();
  }

  bean_temp_filter_.SetMinRunInterval(kFilterRunInterval);
  env_temp_filter_.SetMinRunInterval(kFilterRunInterval);
  ambient_temp_filter_.SetMinRunInterval(kFilterRunInterval);

  Serial1.begin(kSerialBaud);

  transfer_in_.begin(&Serial1);
  transfer_out_.begin(&Serial1);

  // TODO: check sensor state and enable the relay
  Controller::Init();
}

void ArduinoController::Step() {
  digitalWrite(kLed, (millis() / 500) % 2);
  bean_temp_filter_.Run();
  env_temp_filter_.Run();
  ambient_temp_filter_.Run();
  Controller::Step();

  transfer_out_.sendData();
  // Wait for esp32 to send data
  delay(3);
  if (transfer_in_.receiveData()) {
    ReceiveCommand(command_);
    no_comms_timer_.Reset();
  } else if (no_comms_timer_.Expired()) {
    status_.fault.no_comms = true;
  }
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

void ArduinoController::SetHeater(bool on) {
  Controller::SetHeater(on);
  digitalWrite(kSsrEn, on);
}

void ArduinoController::SetStir(bool on) {
  Controller::SetStir(on);
  digitalWrite(kStirEn, on);
}

void ArduinoController::SetRelay(bool on) {
  Controller::SetRelay(on);
  digitalWrite(kRelayEn, on);
}

float ArduinoController::ReadBeanTempF() {
  return bean_temp_filter_.GetFilteredValue();
}

float ArduinoController::ReadEnvTempF() {
  return env_temp_filter_.GetFilteredValue();
}

float ArduinoController::ReadAmbientTempF() {
  return ambient_temp_filter_.GetFilteredValue();
}

uint8_t ArduinoController::BeanTempReadError() {
  return bean_therm_.GetReadError();
}

uint8_t ArduinoController::EnvTempReadError() {
  return env_therm_.GetReadError();
}
