#pragma once

#ifdef ARDUINO

#include <Arduino.h>

#else

#include <cstdint>

uint16_t analogRead(int pin);
void analogReadResolution(int resolution);
void SetAnalogRead(int pin, uint16_t value);

#endif

// Platform-independent declarations

struct RunnerFault {
  bool bean_temp_low : 1;
  bool bean_temp_high : 1;
  bool env_temp_low : 1;
  bool env_temp_high : 1;
  bool ambient_temp_low : 1;
  bool ambient_temp_high : 1;

  bool Faulty() {
    return bean_temp_low || bean_temp_high || env_temp_low || env_temp_high ||
           ambient_temp_low || ambient_temp_high;
  }
};

struct RunnerStatus {
  RunnerFault fault;
};