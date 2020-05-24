#pragma once
#include "types.h"
#include <iostream>

class Channel {
public:
  bool enabled;
  byte volume;
};

class APU;

class Voice {
public:
  bool enabled;
  bool left;
  bool right;
  
  virtual void tick() = 0;
  virtual void tick_length() {
    if (length > 0) --length;
    if (length == 0) {
      enabled = false;
    }
  }
  virtual void tick_volume() {
    if (volume_period > 0) {
      if (volume_timer > 0) {
        --volume_timer;
      }
      
      if (volume_timer == 0) {
        if (increasing && volume < 0xf) {
          ++volume;
        } else if (!increasing && volume > 0x0) {
          --volume;
        }
      }
      
      volume_timer = volume_period;
    }
  }
  
  virtual void tick_sweep() {}
  
  virtual int16_t operator()() = 0;
  
  friend class APU;
  
protected:
  byte length;
  bool counter_selection;
  
  byte initial_volume;
  bool increasing;
  
  byte volume;
  byte volume_period;
  byte volume_timer;
};
