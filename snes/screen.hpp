#pragma once

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <gflags/gflags.h>
#include <array>

#include "types.h"

class SPPU;

class Screen {
public:
  struct colour_t {
    byte r: 5;
    byte g: 5;
    byte b: 5;
    byte unused: 1;
  };

  Screen() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    window_ = SDL_CreateWindow("Screen", 500, 0, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE,
                               SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED
                                                | SDL_RENDERER_PRESENTVSYNC
                                                | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, SCREEN_WIDTH,
                                 SCREEN_HEIGHT);

    font_ = TTF_OpenFont("mplus-2c-medium.ttf", 12);

    SDL_ShowWindow(window_);
  }

  ~Screen() {
    TTF_CloseFont(font_);

    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
  }

  void blit();

  void connect(SPPU* ppu);

//private:
  constexpr static int SCREEN_WIDTH = 256;
  constexpr static int SCREEN_HEIGHT = 224;
  constexpr static int SCALE = 2;

  std::array<byte, (SCREEN_WIDTH * SCREEN_HEIGHT) * 4> buf {};
  std::array<
      std::array<colour_t, SCREEN_WIDTH * SCREEN_HEIGHT>,
      4> fb {};

  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  TTF_Font* font_;

  SPPU* ppu;
};