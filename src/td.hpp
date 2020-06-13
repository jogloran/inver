#pragma once

#include "types.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <gflags/gflags.h>
#include <array>

class PPU;

class TD {
public:
  PPU* ppu;
  TD() {
    SDL_Init(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("TD", 500, 0, TD_WIDTH * SCALE, TD_HEIGHT * SCALE, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TD_WIDTH * SCALE, TD_HEIGHT * SCALE);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TD_WIDTH, TD_HEIGHT);
  }
  
  void connect_ppu(PPU* p) {
    ppu = p;
  }
  
  void show();
  
private:
  constexpr static int TD_WIDTH = 2*32*8;
  constexpr static int TD_HEIGHT = 2*30*8;
  constexpr static int SCALE = 2;
  
  std::array<byte, (TD_WIDTH * TD_HEIGHT) * 4> buf;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
};
