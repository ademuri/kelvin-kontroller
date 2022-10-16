#pragma once

#include <types.h>

class Controller {
 public:
  Controller() {}

  virtual void SetFan(uint8_t pwm);

 protected:
};
