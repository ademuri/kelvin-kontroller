#pragma once

// clang-format off
#include "types.h"
// clang-format on

#include <arduino-timer.h>

class Controller {
 protected:
  RunnerStatus status_{};

 public:
  Controller();
  virtual ~Controller();

  RunnerStatus GetStatus() const { return status_; }

  // Runs one iteration of the control loop
  virtual void Step();

  // Initialize this controller
  virtual void Init();

  // Actuators
  virtual void SetFan(uint8_t pwm);
  virtual void SetHeater(uint8_t pwm);
  virtual void SetStir(bool on);

  // Returns the current actual setpoint for the heater
  uint8_t GetFanValue() const { return fan_value_; }

  // Returns the current actual setpoint for the heater
  uint8_t GetHeaterValue() const { return heater_value_; }

  // Returns the value of the relay, which controls power to the SSR
  bool GetRelayValue() const { return relay_value_; }

  // Sensors
  // Reads the thermocouple for the bean mass
  virtual float ReadBeanTempF() = 0;

  // Reads the thermocouple for the roaster environment (heater output)
  virtual float ReadEnvTempF() = 0;

  // Reads the thermistor on the control board.
  virtual float ReadAmbientTempF() = 0;

 protected:
  virtual void SetRelay(bool on);

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

 private:
  // Safety constants
  // If the ambient is below this, ambient sensing is probably broken
  static constexpr float kMinAmbientTemp = 40;
  // If the ambient is above this, the SSR may overheat
  static constexpr float kMaxAmbientTemp = 120;

  static constexpr float kMinBeanTemp = kMinAmbientTemp;
  // If the beans are above this temp, they are likely to start on fire
  static constexpr float kMaxBeanTemp = 500;

  static constexpr float kMinEnvTemp = kMinAmbientTemp;
  // TODO: tune this based on thermocouple placement
  static constexpr float kMaxEnvTemp = 800;
};
