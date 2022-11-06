#pragma once

// clang-format off
#include "types.h"
// clang-format on

#include <AutoPID.h>
#include <arduino-timer.h>

class Controller {
 protected:
  RunnerStatus status_{};

 public:
  Controller();
  virtual ~Controller();

  RunnerStatus GetStatus() const { return status_; }

  // Clears any faults in the status
  void ResetStatus() { status_ = {}; }

  // Initialize this controller
  virtual void Init();

  // Runs one iteration of the control loop
  virtual void Step();

  void ReceiveCommand(const RunnerCommand &command);

  // Actuators
  void SetFanTarget(uint8_t pwm);
  virtual void SetFan(uint8_t pwm);
  virtual void SetHeater(bool on);
  virtual void SetStir(bool on);

  // Returns the current actual setpoint for the heater
  uint8_t GetFanValue() const { return fan_value_; }

  // Returns the current actual setpoint for the heater
  bool GetHeaterValue() const { return heater_value_; }

  // Returns whether the stir motor is enabled
  bool GetStirValue() const { return stir_value_; }

  // Returns the value of the relay, which controls power to the SSR
  bool GetRelayValue() const { return relay_value_; }

  // Sensors
  // Reads the thermocouple for the bean mass
  virtual float ReadBeanTempF() = 0;

  // Reads the thermocouple for the roaster environment (heater output)
  virtual float ReadEnvTempF() = 0;

  // Reads the thermistor on the control board.
  virtual float ReadAmbientTempF() = 0;

  virtual uint8_t BeanTempReadError() = 0;
  virtual uint8_t EnvTempReadError() = 0;

 protected:
  virtual void SetRelay(bool on);

  uint8_t fan_target_ = 0;
  uint8_t fan_value_ = 0;
  bool heater_value_ = 0;
  bool stir_value_ = 0;
  float set_temp_ = 0;

  bool relay_value_ = 1;

  uint8_t fan_min_ = 150;
  uint8_t fan_max_ = 255;

 private:
  // Period for the SSR PWM output. Should be an even multiple of half-cycles.
  // The SSR is zero-cross output, so for 60Hz mains, there are 120 half-cycles
  // per second. A longer period means more granularity in controlling the heat,
  // at the expense of a slower response time.
  // TODO: tune this
  static constexpr uint32_t kSsrPeriod = 500;
  // TODO: tune this, or possibly remove it if loop timing is consistent
  static constexpr uint32_t kPidPeriod = 80;
  // If the target temp is farther than this from the setpoint, use simple
  // on-off control.
  static constexpr float kBangBangThreshold = 0;

  // TODO: tune these
  float p = 0.1;
  float i = 0;
  float d = 0;
  AutoPIDRelay temp_pid{kSsrPeriod, p, i, d};

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

  // When the bean temp is above this, stir should be enabled.
  static constexpr float kStirTemp = 120;

  // If the beans are above this temp, fan should be on
  static constexpr float kBeanFanTemp = 180;

  // If the heater are above this temp, fan should be on
  static constexpr float kEnvFanTemp = 140;
};
