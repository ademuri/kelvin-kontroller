#include "controller.h"

#include <algorithm>

Controller::Controller() {
}

Controller::~Controller() {}

void Controller::Init() { SetRelay(true); }

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

  if ((uint32_t)status_.fault.Faulty()) {
    if (relay_value_ == 1) {
      // When a fault occurs, fail safe: disable the heater, turn the fan to
      // maximum, and enable the stir.
      SetFan(255);
      SetHeater(0);
      SetStir(true);
      SetRelay(false);

      // TODO: turn on the buzzer
    }

    // If a fault has already occurred, just read the temps and do nothing else.
    return;
  }
}

void Controller::SetFan(uint8_t pwm) {
  fan_target_ = pwm;
  fan_value_ = std::min(fan_max_, pwm);
  fan_value_ = std::max(fan_min_, pwm);
}

void Controller::SetHeater(uint8_t pwm) {
  heater_target_ = pwm;
  heater_value_ = std::min(heater_max_, pwm);
  heater_value_ = std::max(heater_min_, pwm);
}

void Controller::SetStir(bool on) {
  stir_value_ = on;
}

void Controller::SetRelay(bool on) {
  relay_value_ = on;
}
