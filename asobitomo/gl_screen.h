#pragma once

#include "screen.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>
#include <gflags/gflags.h>

DECLARE_bool(limit_framerate);
DECLARE_int32(us_per_frame);

class CPU;

class GL : public Screen {
public:
  CPU& cpu_;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  GL(CPU& cpu, int scale=4);
  
  void blit();
  
  std::array<byte, BUF_WIDTH * BUF_HEIGHT * 4> buf;
  int scale_;
  
  std::chrono::time_point<std::chrono::high_resolution_clock> last_;
  
  enum class Speed {
    Fast,
    Normal,
    Slow
  };
  Speed speed;
  bool speed_toggled;
};
