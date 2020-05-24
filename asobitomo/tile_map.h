#pragma once

#include "types.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <gflags/gflags.h>
#include <array>

DECLARE_bool(tm);

class PPU;

class TM {
public:
  TM(PPU& ppu): ppu_(ppu), enabled_(FLAGS_tm), scale_(2) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("TM", 0, 500, TM_WIDTH * scale_, TM_HEIGHT * scale_, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TM_WIDTH * scale_, TM_HEIGHT * scale_);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TM_WIDTH, TM_HEIGHT);
//    SDL_MinimizeWindow(window_);
  }
  
  void set_enabled(bool enabled) {
    enabled_ = enabled;
    std::cout << "tm enabled: " << enabled_ << std::endl;
    if (enabled_) {
      SDL_ShowWindow(window_);
    } else {
      SDL_HideWindow(window_);
    }
  }
  
private:
  constexpr static int TM_WIDTH = 32*8;
  constexpr static int TM_HEIGHT = 32*8;
  
  PPU& ppu_;
  int scale_;
  
  std::array<byte, (TM_WIDTH * 4) * (TM_HEIGHT * 4)> buf;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;

  bool enabled_;
};
