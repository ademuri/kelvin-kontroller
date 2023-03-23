#pragma once

#include "types.h"

class Ramper {
 public:
  Ramper(uint32_t period, uint16_t max_change)
      : period_(period), max_change_(max_change) {}

  void SetTarget(uint16_t target);
  uint16_t Step();
  uint16_t Get() const { return actual_; }

 private:
  uint16_t target_ = 0;
  uint16_t actual_ = 0;
  uint32_t update_at_ms_ = 0;

  const uint32_t period_;
  const uint16_t max_change_;
};
