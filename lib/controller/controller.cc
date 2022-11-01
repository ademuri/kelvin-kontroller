#include "controller.h"

#include <algorithm>

Controller::Controller() {}

Controller::~Controller() {}

void Controller::Init() {
  SetRelay(true);
  temp_pid.setTimeStep(kPidPeriod);
  temp_pid.setBangBang(kBangBangThreshold);
}

void Controller::Step() {
  const float bean_temp = ReadBeanTempF();
  const float env_temp = ReadEnvTempF();
  const float ambient_temp = ReadAmbientTempF();

  if (bean_temp < kMinBeanTemp) {
    status_.fault.bean_temp_low = 1;
  }
  if (bean_temp > kMaxBeanTemp) {
    status_.fault.bean_temp_high = 1;
  }

  if (env_temp < kMinEnvTemp) {
    status_.fault.env_temp_low = 1;
  }
  if (env_temp > kMaxEnvTemp) {
    status_.fault.env_temp_high = 1;
  }

  if (ambient_temp < kMinAmbientTemp) {
    status_.fault.ambient_temp_low = 1;
  }
  if (ambient_temp > kMaxAmbientTemp) {
    status_.fault.ambient_temp_high = 1;
  }

  status_.fault.bean_temp_read_error =
      std::max(status_.fault.bean_temp_read_error, BeanTempReadError());
  status_.fault.env_temp_read_error =
      std::max(status_.fault.env_temp_read_error, EnvTempReadError());

  status_.bean_temp = bean_temp;
  status_.env_temp = env_temp;
  status_.ambient_temp = ambient_temp;
  status_.fan_speed = GetFanValue();

  if (status_.fault.Faulty()) {
    if (relay_value_ == 1) {
      // When a fault occurs, fail safe: disable the heater, turn the fan to
      // maximum, and enable the stir.
      SetFan(255);
      SetHeater(false);
      SetStir(true);
      SetRelay(false);

      // TODO: turn on the buzzer
    }

    // If a fault has already occurred, just read the temps and do nothing else.
    return;
  }

  temp_pid.run(bean_temp);
  SetHeater(temp_pid.getRelayState());
}

void Controller::ReceiveCommand(const RunnerCommand &command) {
  temp_pid.setSetPoint(command.target_temp);
  SetFan(command.fan_speed);
  if (command.reset == true) {
    ResetStatus();
  }
}

void Controller::SetFan(uint8_t pwm) {
  fan_target_ = pwm;
  fan_value_ = std::min(fan_max_, pwm);
  fan_value_ = std::max(fan_min_, pwm);
}

void Controller::SetHeater(bool on) { heater_value_ = on; }

void Controller::SetStir(bool on) { stir_value_ = on; }

void Controller::SetRelay(bool on) { relay_value_ = on; }
