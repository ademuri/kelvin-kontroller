#pragma once

#include <Arduino.h>
#include <EasyTransfer.h>
#include <arduino-timer.h>
#include <median-filter.h>

#include "controller.h"
#include "max31855-thermocouple.h"
#include "thermistor.h"
#include "types.h"

class ArduinoController : public Controller {
 public:
  ArduinoController() : Controller() {}

  // Initializes peripherals. Call once before using.
  void Init() override;

  // Runs one cycle of the control loop. Call periodically.
  void Step() override;

  void SetFan(uint8_t pwm) override;
  void SetHeater(bool on) override;
  void SetStir(bool on) override;
  void SetRelay(bool on) override;

  static constexpr int kSpiSck = PA5;
  static constexpr int kSpiMiso = PA6;

  float ReadBeanTempF() override;
  float ReadEnvTempF() override;
  float ReadAmbientTempF() override;
  uint8_t BeanTempReadError() override;
  uint8_t EnvTempReadError() override;

 private:
  static constexpr int kSpiNss1 = PB0;
  static constexpr int kSpiNss2 = PB1;

  static constexpr int kEspUsartRx = PA9;
  static constexpr int kEspUsartTx = PA10;

  static constexpr bool kForceDisableFan = true;
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

  Max31855Thermocouple bean_therm_{kSpiNss1};
  Max31855Thermocouple env_therm_{kSpiNss2};
  Thermistor ambient_therm_{kThermistor2, /*analog_reference_volts=*/3.3};

  static constexpr uint32_t kFilterSize = 7;
  MedianFilter<float, float, kFilterSize> bean_temp_filter_ =
      MedianFilter<float, float, kFilterSize>(
          [this]() { return bean_therm_.ReadFahrenheit(); });
  MedianFilter<float, float, kFilterSize> env_temp_filter_ =
      MedianFilter<float, float, kFilterSize>(
          [this]() { return env_therm_.ReadFahrenheit(); });
  MedianFilter<float, float, kFilterSize> ambient_temp_filter_ =
      MedianFilter<float, float, kFilterSize>(
          [this]() { return ambient_therm_.ReadFahrenheit(); });

  // Note: the MAX31855 takes 70-100ms to convert a temperature, and it converts
  // continuously.
  static constexpr uint32_t kFilterRunInterval = 80;

  RunnerCommand command_;
  EasyTransfer<RunnerCommand> transfer_in_{&command_};
  EasyTransfer<RunnerStatus> transfer_out_{&status_};

  static constexpr uint32_t kNoCommsTimeout = 10 * 1000;
  CountDownTimer no_comms_timer_{kNoCommsTimeout};

//   HardwareTimer buzzer_timer_(TIM16);
};
