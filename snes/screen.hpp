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
  union colour_t {
    struct {
      byte r: 5;
      byte g: 5;
      byte b: 5;
      byte unused: 1;
    };
    word reg;

    template<typename Ar>
    void serialize(Ar& ar) {
      ar(reg);
    }

    colour_t operator+(const colour_t& other) {
      return {
          .r = static_cast<byte>(std::max((this->r + other.r), 31)),
          .g = static_cast<byte>(std::max((this->g + other.g), 31)),
          .b = static_cast<byte>(std::max((this->b + other.b), 31))};
    }

    colour_t operator-(const colour_t& other) {
      return {
          .r = static_cast<byte>(this->r < other.r ? 0 : this->r - other.r),
          .g = static_cast<byte>(this->g < other.r ? 0 : this->g - other.g),
          .b = static_cast<byte>(this->b < other.b ? 0 : this->b - other.b)};
    }

    colour_t div2() {
      return {
          .r = static_cast<byte>(this->r / 2),
          .g = static_cast<byte>(this->g / 2),
          .b = static_cast<byte>(this->b / 2)};
    }
  };

  SDL_Texture* make_raster_texture(size_t dx, size_t dy) {
    SDL_Texture* raster = SDL_CreateTexture(renderer_, SDL_GetWindowPixelFormat(window_),
                                            SDL_TEXTUREACCESS_TARGET,
                                            SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE);
    SDL_SetRenderTarget(renderer_, raster);
    SDL_SetTextureBlendMode(raster, SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
    SDL_RenderClear(renderer_);

    SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);

    for (int x = 0; x < OUTPUT_WIDTH; x += dx * SCALE / 2) {
      SDL_RenderDrawLine(renderer_, x, 0, x, OUTPUT_HEIGHT);
    }

    for (int y = 0; y < OUTPUT_HEIGHT; y += dy * SCALE / 2) {
      SDL_RenderDrawLine(renderer_, 0, y, OUTPUT_WIDTH, y);
    }

    SDL_SetRenderTarget(renderer_, nullptr);

    return raster;
  }

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
    raster_ = make_raster_texture(8, 8);

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

  void connect(std::shared_ptr<SPPU> ppu);

//private:
  constexpr static int SCREEN_WIDTH = 256;
  constexpr static int SCREEN_HEIGHT = 224;
  constexpr static int SCALE = 2;
  static constexpr int OUTPUT_WIDTH = SCREEN_WIDTH * SCALE;
  static constexpr int OUTPUT_HEIGHT = SCREEN_HEIGHT * SCALE;

  std::array<byte, (SCREEN_WIDTH * SCREEN_HEIGHT) * 4> buf {};
  std::array<
      std::array<colour_t, SCREEN_WIDTH * SCREEN_HEIGHT>,
      4> fb {};

  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  TTF_Font* font_;
  SDL_Texture* raster_;

  std::shared_ptr<SPPU> ppu;

  void set_brightness(byte b);

  byte brightness;
};