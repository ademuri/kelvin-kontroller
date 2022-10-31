#pragma once

#include <Arduino.h>

#include <cstdint>

class Programmer {
  Programmer() {}

  size_t sendData(const uint8_t *data, int len);
};
