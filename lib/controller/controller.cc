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
  // if (bean_temp < kMinBeanTemp) {
  //   status_.fault_since_reset.bean_temp_low = 1;
  //   temp_out_of_range_ = true;
  // }
  // if (bean_temp > kMaxBeanTemp) {
  //   status_.fault_since_reset.bean_temp_high = 1;
  //   temp_out_of_range_ = true;
  // }

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
  fan_ramper_.Step();
  status_.fan_speed = GetFanValue();

  fault_filter_.Run();

  if (fault_filter_.GetFilteredValue() == 0) {
    status_.fatal_fault = false;
  } else {
    status_.fatal_fault = true;
    // When a fault occurs, fail safe: disable the heater, turn the fan to
    // maximum, and enable the stir.
    SetFan(255);
    SetHeater(false);
    SetStir(true);
    SetRelay(false);

    // If a fault has already occurred, just read the temps and do nothing else.
    return;
  }

  set_temp_ramper_.Step();
  temp_pid.setSetPoint(set_temp_ramper_.Get());
  status_.target_temp = set_temp_ramper_.Get();
  temp_pid.run(env_temp);
  if (fan_target_ > 0 && fan_ramper_.Get() > 0) {
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
  set_temp_ = command.target_temp;
  set_temp_ramper_.SetTarget(command.target_temp);
  temp_pid.setSetPoint(set_temp_ramper_.Get());
  status_.target_temp = set_temp_ramper_.Get();
  temp_pid.setManualOutput(command.manual_output);

  if (!status_.fatal_fault) {
    SetFanTarget(command.fan_speed);
  }
  if (command.reset == true) {
    ResetStatus();
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
  uint8_t wanted_fan_value;
  if (pwm > 0) {
    wanted_fan_value = std::min(fan_max_, pwm);
    wanted_fan_value = std::max(fan_min_, pwm);
    if (!fan_timer_.Running()) {
      fan_timer_.Reset();
    }
  } else {
    wanted_fan_value = 0;
    fan_timer_.Stop();
  }
  fan_ramper_.SetTarget(wanted_fan_value);
}

void Controller::SetHeater(bool on) {
  if (fan_timer_.Get() > kFanToHeaterOnDelay) {
    heater_value_ = on;
  } else {
    heater_value_ = false;
  }
}

void Controller::SetStir(bool on) { stir_value_ = on; }

void Controller::SetRelay(bool on) { relay_value_ = on; }
