#include "controller.h"

#include <algorithm>

Controller::Controller() {}

Controller::~Controller() {}

void Controller::Init() {
  SetRelay(true);
  temp_pid.setTimeStep(kPidPeriod);
  temp_pid.setBangBang(kBangBangThreshold);
  fault_filter_.SetMinRunInterval(100);
}

void Controller::Step() {
  const float bean_temp = ReadBeanTempF();
  const float env_temp = ReadEnvTempF();
  const float ambient_temp = ReadAmbientTempF();

  if (millis() < kTurnOnDelay) {
    return;
  }

  temp_out_of_range_ = false;
  if (bean_temp < kMinBeanTemp) {
    status_.fault_since_reset.bean_temp_low = 1;
    temp_out_of_range_ = true;
  }
  if (bean_temp > kMaxBeanTemp) {
    status_.fault_since_reset.bean_temp_high = 1;
    temp_out_of_range_ = true;
  }

  if (env_temp < kMinEnvTemp) {
    status_.fault_since_reset.env_temp_low = 1;
    temp_out_of_range_ = true;
  }
  if (env_temp > kMaxEnvTemp) {
    status_.fault_since_reset.env_temp_high = 1;
    temp_out_of_range_ = true;
  }

  if (ambient_temp < kMinAmbientTemp) {
    status_.fault_since_reset.ambient_temp_low = 1;
    temp_out_of_range_ = true;
  }
  if (ambient_temp > kMaxAmbientTemp) {
    status_.fault_since_reset.ambient_temp_high = 1;
    temp_out_of_range_ = true;
  }

  status_.fault_since_reset.bean_temp_read_error = std::max(
      status_.fault_since_reset.bean_temp_read_error, BeanTempReadError());
  status_.fault_since_reset.env_temp_read_error = std::max(
      status_.fault_since_reset.env_temp_read_error, EnvTempReadError());

  status_.bean_temp = bean_temp;
  status_.env_temp = env_temp;
  status_.ambient_temp = ambient_temp;
  status_.fan_speed = GetFanValue();

  fault_filter_.Run();

  if (fault_filter_.GetFilteredValue() != 0 || status_.fatal_fault) {
    status_.fatal_fault = true;
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

  temp_pid.run(env_temp);
  if (fan_target_ > 0 && fan_value_ > 0) {
    status_.heater_output = temp_pid.getOutput();
    SetHeater(temp_pid.getRelayState());
  } else {
    status_.heater_output = 0;
    SetHeater(false);
    if ((bean_temp > kBeanFanTemp || env_temp > kEnvFanTemp) &&
        fan_target_ == 0) {
      SetFan(255);
    } else {
      SetFan(fan_target_);
    }
  }

  if (bean_temp > kStirTemp || set_temp_ > 0) {
    SetStir(true);
  } else {
    SetStir(false);
  }
}

void Controller::ReceiveCommand(const RunnerCommand &command) {
  if (command.heater_duty != 0) {
    set_temp_ = 0;
  } else {
    set_temp_ = command.target_temp;
    temp_pid.setSetPoint(command.target_temp);
  }

  SetFanTarget(command.fan_speed);
  if (command.reset == true) {
    ResetStatus();
    fault_filter_
  }
  if (command.p >= 0) {
    p = command.p;
  }
  if (command.i >= 0) {
    i = command.i;
  }
  if (command.d >= 0) {
    d = command.d;
  }
  temp_pid.setGains(p, i, d);
}

void Controller::SetFanTarget(uint8_t pwm) {
  fan_target_ = pwm;
  SetFan(fan_target_);
}

void Controller::SetFan(uint8_t pwm) {
  if (pwm > 0) {
    fan_value_ = std::min(fan_max_, pwm);
    fan_value_ = std::max(fan_min_, pwm);
  } else {
    fan_value_ = 0;
  }
}

void Controller::SetHeater(bool on) { heater_value_ = on; }

void Controller::SetStir(bool on) { stir_value_ = on; }

void Controller::SetRelay(bool on) { relay_value_ = on; }
