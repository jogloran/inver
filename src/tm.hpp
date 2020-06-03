#pragma once

#include "types.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <gflags/gflags.h>
#include <array>

class PPU;

class TM {
public:
  PPU* ppu;
  TM() {
    SDL_Init(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("TM", 500, 0, TM_WIDTH * 4, TM_HEIGHT * 4, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TM_WIDTH * 4, TM_HEIGHT * 4);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TM_WIDTH, TM_HEIGHT);
  }
  
  void connect_ppu(PPU* p) {
    ppu = p;
  }
  
  void show();
  
private:
  constexpr static int TM_WIDTH = 32*8;
  constexpr static int TM_HEIGHT = 16*8;
  
  std::array<byte, (TM_WIDTH * TM_HEIGHT) * 4> buf;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
};
