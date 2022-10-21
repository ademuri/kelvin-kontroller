#include "controller.h"

Controller::Controller() {
  bean_temp_filter_.SetMinRunInterval(10);
  env_temp_filter_.SetMinRunInterval(10);
  ambient_temp_filter_.SetMinRunInterval(10);
}

Controller::~Controller() {}

void Controller::Step() {
  bean_temp_filter_.Run();
  env_temp_filter_.Run();
  ambient_temp_filter_.Run();

  const float bean_temp = bean_temp_filter_.GetFilteredValue();
  const float env_temp = env_temp_filter_.GetFilteredValue();
  const float ambient_temp = ambient_temp_filter_.GetFilteredValue();
}