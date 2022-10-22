#pragma once

#include "controller.h"

class FakeController : public Controller {
 private:
  float bean_temp_ = 0;
  float env_temp_ = 0;
  float ambient_temp_ = 0;
  uint8_t bean_temp_read_error_ = 0;
  uint8_t env_temp_read_error_ = 0;

 public:
  float ReadBeanTempF() override { return bean_temp_; }
  float ReadEnvTempF() override { return env_temp_; }
  float ReadAmbientTempF() override { return ambient_temp_; }
  void SetBeanTempF(float temp) { bean_temp_ = temp; }
  void SetEnvTempF(float temp) { env_temp_ = temp; }
  void SetAmbientTempF(float temp) { ambient_temp_ = temp; }
  uint8_t BeanTempReadError() override { return bean_temp_read_error_; }
  uint8_t EnvTempReadError() override { return env_temp_read_error_; }
  void SetBeanTempReadError(uint8_t error) { bean_temp_read_error_ = error; }
  void SetEnvTempReadError(uint8_t error) { env_temp_read_error_ = error; }
};
