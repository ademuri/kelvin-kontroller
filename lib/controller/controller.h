#pragma once

// clang-format off
#include "types.h"
// clang-format on

#include <AutoPID.h>
#include <arduino-timer.h>
#include <median-filter.h>

#include "ramper.h"

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
  uint8_t GetFanValue() const { return fan_ramper_.Get(); }

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

  bool heater_value_ = 0;
  bool stir_value_ = 0;
  float set_temp_ = 0;

  bool relay_value_ = 1;

  uint8_t fan_min_ = 50;
  uint8_t fan_max_ = 255;

 private:
  uint8_t fan_target_ = 0;
  Ramper fan_ramper_{/*period=*/5, /*max_change=*/10};

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

  bool temp_out_of_range_ = false;

  // TODO: tune these
  float p = 0.1;
  float i = 0;
  float d = 0;
  Ramper set_temp_ramper_{/*period=*/500, /*max_change*/1};
  AutoPIDRelay temp_pid{kSsrPeriod, p, i, d};

  CountUpTimer fan_timer_;

  // Safety constants
  // If the ambient is below this, ambient sensing is probably broken
  static constexpr float kMinAmbientTemp = 30;
  // If the ambient is above this, the SSR may overheat
  static constexpr float kMaxAmbientTemp = 120;

  static constexpr float kMinBeanTemp = kMinAmbientTemp;
  // If the beans are above this temp, they are likely to start on fire
  static constexpr float kMaxBeanTemp = 600;

  static constexpr float kMinEnvTemp = kMinAmbientTemp;
  // TODO: tune this based on thermocouple placement
  static constexpr float kMaxEnvTemp = 1200;

  // When the bean temp is above this, stir should be enabled.
  static constexpr float kStirTemp = 120;

  // If the beans are above this temp, fan should be on
  static constexpr float kBeanFanTemp = 180;

  // If the heater is above this temp, fan should be on
  static constexpr float kEnvFanTemp = 140;

  // Allow sensors to stabilize before enabling the control loop
  static constexpr uint32_t kTurnOnDelay = 2000;

  // Wait for the fan to be running for this long before turning on the heater.
  // This provides safety margin, so that the fan spins up first.
  static constexpr uint32_t kFanToHeaterOnDelay = 1000;

 public:
  // Public for testing, so that tests can set millis on this.
  MedianFilter<uint32_t, uint32_t, 9> fault_filter_ =
      MedianFilter<uint32_t, uint32_t, 9>([this]() {
        return temp_out_of_range_ || status_.fault_since_reset.no_comms ||
               //BeanTempReadError() ||
               EnvTempReadError();
      });

  // Public for testing
  uint8_t GetFanTarget() { return fan_target_; }
};
