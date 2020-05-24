#pragma once

#include <array>
#include <iostream>

class Noise : public Voice {
public:
  Noise(): noise_reg(0x7fff) {}
  
  void trigger() {
    enabled = true;
    if (length == 0) {
      length = 64;
    }
    volume = initial_volume;
    volume_timer = volume_period;
    timer = divisor() << shift_clock_frequency;
    
    noise_reg = 0x7fff;
  }
  
  byte divisor() {
    static byte table[] {
      8, 16, 32, 48, 64, 80, 96, 112
    };
    return table[dividing_ratio];
  }
  
  void tick() {
    if (timer > 0) --timer;
    if (timer == 0) {
      byte mash = ((noise_reg & 0x2) != 0) ^ (noise_reg & 0x1);
      noise_reg >>= 1;
      noise_reg |= (mash << 14);
      
      if (!counter_width_is_15) {
        noise_reg |= (mash << 6);
      }
    
      timer = divisor() << shift_clock_frequency;
    }
  }
  
  int16_t operator()() {
    return 0;
    if (!enabled) return 0;
    
    if ((noise_reg & 0x1) != 0) {
      return volume;
    }
    return 0;
  }
  
  void set_length(byte value) {
    values[0] = value;
    length = 64 - (value & 0x1f);
  }
  
  void set_volume_envelope(byte value) {
    values[1] = value;
    volume = initial_volume = value >> 4;
    increasing = (value & 0x8) != 0;
    volume_timer = volume_period = value & 0x7;
  }
  
  void set_polynomial_counter(byte value) {
    values[2] = value;
    
    shift_clock_frequency = value >> 4;
    timer = divisor() << shift_clock_frequency;
    
    counter_width_is_15 = (value & 0x8) == 0;
    dividing_ratio = value & 0x7;
  }
  void set_counter_consecutive(byte value) {
    values[3] = value;
    counter_selection = (value & 0x40) != 0;
    
    if (value & (1 << 7)) {
      trigger();
    }
  }
  
//private:
  word noise_reg; // 15 bits
  word timer;
  
  byte initial_volume;
  bool increasing;
  
  byte shift_clock_frequency;
  bool counter_width_is_15;
  byte dividing_ratio;
  
  bool counter_selection;
  
  std::array<byte, 4> values;
};
