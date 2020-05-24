#pragma once

#include <array>
#include <iostream>

class Wave : public Voice {
public:
  void tick() {
    if (timer > 0) --timer;
    if (timer == 0) {
      wave_position = (wave_position + 1) % 32;
      
      buffer = wave[wave_position / 2];
      buffer >>= (1 - (wave_position % 2)) * 4;
      buffer &= 0xf;
      
      timer = (2048 - freq()) * 2;
    }
    
//      // Fill the sample buffer
//      //  <position> / 2 = wave index
//      self.Buffer = *(self.RAM + (self.Position / 2));
//      //  <position> % 2 = 0 (hi) or 1 (lo)
//      self.Buffer >>= (1 - (self.Position % 2)) * 4;
//      self.Buffer &= 0x0F;
//
//      // Reload timer
//      self.Timer = (2048 - self.Frequency) * 2;
//    }
  }
  
  virtual void tick_volume() {}
  int16_t operator()() {
    if (!(enabled && dac_enabled)) return 0;
    
    return buffer >> 2;
  }
  
  inline word freq() {
    return (freq_hi << 8) | freq_lo;
  }
  
  virtual void tick_length() {
    if (length_word > 0) --length_word;
    if (length_word == 0) {
      enabled = false;
    }
  }
  
  void trigger() {
    enabled = true;
    if (length_word == 0) {
      length_word = 256;
    }
    timer = (2048 - freq()) * 2;
    wave_position = 0;
  }
  
  void set_enabled(byte value) {
    values[0] = value;
    dac_enabled = (value & 0x80) != 0;
  }
  
  void set_length(byte value) {
    values[1] = value;
    length = value;
  }
  
  void set_output_level(byte value) {
    values[2] = value;
    output_level = value;
  }
  
  void set_wave_low(byte value) {
    values[3] = value;
    freq_lo = value;
  }
  
  void set_wave_high_and_control(byte value) {
    values[4] = value;
    freq_hi = value & 0x7;
  }
  
  void set_wave_pattern(byte value, byte offset) {
    wave[offset] = value;
  }
  
//private:
  byte wave_position;
  
  std::array<byte, 5> values;
  
  std::array<byte, 32> wave;
  bool dac_enabled;
  byte output_level;
  
  byte freq_lo;
  byte freq_hi; // 1 high bit
  
  word timer;
  word length_word;
  
  byte buffer;
};
