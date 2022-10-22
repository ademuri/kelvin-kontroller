#include "controller.h"

Controller::Controller() {
  bean_temp_filter_.SetMinRunInterval(kFilterRunInterval);
  env_temp_filter_.SetMinRunInterval(kFilterRunInterval);
  ambient_temp_filter_.SetMinRunInterval(kFilterRunInterval);
}

Controller::~Controller() {}

void Controller::Init() { SetRelay(true); }

void Controller::Step() {
  bean_temp_filter_.Run();
  env_temp_filter_.Run();
  ambient_temp_filter_.Run();

  const float bean_temp = bean_temp_filter_.GetFilteredValue();
  const float env_temp = env_temp_filter_.GetFilteredValue();
  const float ambient_temp = ambient_temp_filter_.GetFilteredValue();

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