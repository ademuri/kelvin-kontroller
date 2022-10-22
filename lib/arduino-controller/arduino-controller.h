#pragma once

#include <Arduino.h>

#include "controller.h"
#include "types.h"

class ArduinoController : public Controller {
 public:
  ArduinoController() : Controller() {}

  // Initializes peripherals. Call once before using.
  void Init() override;

  // Runs one cycle of the control loop. Call periodically.
  void Step() override;

  void SetFan(uint8_t pwm) override;
  void SetHeater(uint8_t pwm) override;
  void SetStir(bool on) override;
  void SetRelay(bool on) override;

  static constexpr int kSpiSck = PA5;
  static constexpr int kSpiMiso = PA6;

 protected:
  float ReadBeanTempF() override;
  float ReadEnvTempF() override;
  float ReadAmbientTempF() override;

 private:
  static constexpr int kSpiNss1 = PB0;
  static constexpr int kSpiNss2 = PB1;

  static constexpr int kEspUsartRx = PA9;
  static constexpr int kEspUsartTx = PA10;

  static constexpr bool kForceDisableFan = false;
  static constexpr int kFanEn = PA7;
  static constexpr int kFanPwmOut = PA11;
  static constexpr int kFanTach = PD1;

  static constexpr int kStirEn = PA8;
  static constexpr int kSsrEn = PC6;

  static constexpr int kThermistor1 = PB10;
  static constexpr int kThermistor2 = PB11;

  static constexpr int kRelayEn = PD2;

  static constexpr int kBuzzer = PD0;
  static constexpr int kLed = PA4;

  static constexpr int kAux1 = PB6;
  static constexpr int kAux2 = PB7;
  static constexpr int kAux3 = PB8;
  static constexpr int kAuxEn = PC7;
};
