#pragma once

#include "types.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <gflags/gflags.h>
#include <array>

class SPPU;
class BusSNES;

class TD2 {
public:
  SPPU* ppu;
  TD2() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    window_ = SDL_CreateWindow("TD2", 500, 0, TD2_WIDTH * SCALE, TD2_HEIGHT * SCALE, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED
                                                | SDL_RENDERER_PRESENTVSYNC
                                                | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TD2_WIDTH * SCALE, TD2_HEIGHT * SCALE);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TD2_WIDTH, TD2_HEIGHT);

    font_ = TTF_OpenFont("mplus-2c-medium.ttf", 12);

    SDL_ShowWindow(window_);
  }

  ~TD2() {
    TTF_CloseFont(font_);

    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
  }
  
  void connect(BusSNES* b);
  
  void show();
  
private:
  constexpr static int TD2_WIDTH = 2*32*8;
  constexpr static int TD2_HEIGHT = 2*32*8;
  constexpr static int SCALE = 2;
  
  std::array<byte, (TD2_WIDTH * TD2_HEIGHT) * 4> buf {};
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  TTF_Font* font_;

  void draw_text(const char* txt, SDL_Rect rect);
};
