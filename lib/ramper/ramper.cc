#include "ramper.h"

void Ramper::SetTarget(uint16_t target) {
  target_ = target;
  if (target < actual_ ) {
    actual_ = target;
  }
}

uint16_t Ramper::Step() {
  if (actual_ < target_ && millis() >= update_at_ms_) {
    if ((target_ - actual_) < max_change_) {
      actual_ = target_;
    } else {
      actual_ += max_change_;
    }
    update_at_ms_ = millis() + period_;
  }
  
  return actual_;
}
