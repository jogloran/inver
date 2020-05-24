#pragma once

#include "audio_base.h"
#include "square.h"
#include "wave.h"
#include "noise.h"
#include "types.h"

#include <array>
#include <iostream>
#include <SDL2/SDL.h>

class APU {
public:
  APU(): seq(8192), sample_timer(CYCLES_PER_SECOND / SAMPLE_RATE / 2), buf(), buf_ptr(buf.begin()) {
    sdl_setup();
  }
  
  byte* get(word loc);
  bool set(word loc, byte value);
  
  void step(long delta);
  
  void set_channel_control(byte value) {
    right.volume = (value & 0x70) >> 4;
    left.volume = (value & 0x7);
    right.enabled = (value & 0x80) != 0;
    left.enabled = (value & 0x8) != 0;
  }
  
  void set_sound_output(byte value) {
    ch4.right = (value & (1 << 7)) != 0;
    ch3.right = (value & (1 << 6)) != 0;
    ch2.right = (value & (1 << 5)) != 0;
    ch1.right = (value & (1 << 4)) != 0;
    
    ch4.left = (value & (1 << 3)) != 0;
    ch3.left = (value & (1 << 2)) != 0;
    ch2.left = (value & (1 << 1)) != 0;
    ch1.left = (value & (1 << 0)) != 0;
  }
  void set_channel_levels(byte value) {
    global_enabled = (value & 0x80) != 0;
//    ch4.enabled = (value & (1 << 3)) != 0;
//    ch3.enabled = (value & (1 << 2)) != 0;
//    ch2.enabled = (value & (1 << 1)) != 0;
//    ch1.enabled = (value & (1 << 0)) != 0;
  }
  
private:
  void sdl_setup();
  
  Square1 ch1;
  Square2 ch2;
  Wave ch3;
  Noise ch4;
  
  Channel left;
  Channel right;
  
  bool global_enabled;
  
  word seq;
  byte seq_step; // 0-7
  
  word sample_timer;
  int step_count;
  
  constexpr static int CYCLES_PER_SECOND = 4'194'304;
  constexpr static int SAMPLE_RATE = 24'000;
  SDL_AudioDeviceID dev;
  
  constexpr static int BUFSIZE = 1024;
  constexpr static int NCHANNELS = 2;
  std::array<int16_t, BUFSIZE * NCHANNELS> buf;
  std::array<int16_t, BUFSIZE * NCHANNELS>::iterator buf_ptr;
};

