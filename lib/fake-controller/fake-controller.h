#pragma once

#include "controller.h"

class FakeController : public Controller {
 private:
  float bean_temp_ = 0;
  float env_temp_ = 0;
  float ambient_temp_ = 0;

 public:
  float ReadBeanTempF() override { return bean_temp_; }
  float ReadEnvTempF() override { return env_temp_; }
  float ReadAmbientTempF() override { return ambient_temp_; }
  void SetBeanTempF(float temp) { bean_temp_ = temp; }
  void SetEnvTempF(float temp) { env_temp_ = temp; }
  void SetAmbientTempF(float temp) { ambient_temp_ = temp; }
};
