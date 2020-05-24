#pragma once

#include "types.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <gflags/gflags.h>
#include <array>

DECLARE_bool(td);

class PPU;

class TD {
public:
  TD(PPU& ppu): ppu_(ppu), enabled_(FLAGS_td) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("TD", 500, 0, TD_WIDTH * 4, TD_HEIGHT * 4, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TD_WIDTH * 4, TD_HEIGHT * 4);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TD_WIDTH, TD_HEIGHT);
//    SDL_MinimizeWindow(window_);
  }
  
  void show();
  
  void set_enabled(bool enabled) {
    enabled_ = enabled;
    std::cout << "td enabled: " << enabled_ << std::endl;
    
    if (enabled_) {
      SDL_ShowWindow(window_);
    } else {
      SDL_HideWindow(window_);
    }
  }
  
private:
  PPU& ppu_;
  
  std::array<byte, (16*8*4) * (16*8) * 4> buf;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  constexpr static int TD_WIDTH = 16*8;
  constexpr static int TD_HEIGHT = 16*8;
  
  bool enabled_;
};
