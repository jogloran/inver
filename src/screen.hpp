#pragma once

#include <array>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <gflags/gflags.h>
#include <chrono>

#include "types.h"

using namespace std::chrono_literals;

class PPU;

class Bus;

class Screen {
public:
  Screen() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    TTF_Init();
    window_ = SDL_CreateWindow("Game",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               BUF_WIDTH * SCALE, BUF_HEIGHT * SCALE, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED
                                                | SDL_RENDERER_PRESENTVSYNC
                                                | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, BUF_WIDTH * SCALE, BUF_HEIGHT * SCALE);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 BUF_WIDTH, BUF_HEIGHT);
    SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
    font_ = TTF_OpenFont("mplus-2c-medium.ttf", 24);
//    TTF_SetFontOutline(font_, 2);
    raster_ = make_raster_texture(8, 8);

    SDL_ShowWindow(window_);

    std::fill(fb.begin(), fb.end(), 0);
    std::fill(buf.begin(), buf.end(), 0);
  }

  ~Screen() {
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
  }

  void blit();

  static constexpr int BUF_WIDTH = 256;
  static constexpr int BUF_HEIGHT = 240;
  static constexpr int SCALE = 4;
  static constexpr std::chrono::milliseconds MS_PER_FRAME = 1000ms / 60;
  static constexpr std::chrono::milliseconds TOAST_FADE_OUT_TIME = 500ms;

  // Each byte of fb is a palette index (0...0x40)
  std::array<byte, BUF_WIDTH * BUF_HEIGHT> fb;
  std::array<byte, BUF_WIDTH * BUF_HEIGHT * 4> buf;
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  TTF_Font* font_;
  std::string text_;
  SDL_Texture* text_texture_;
  SDL_Texture* raster_;
  std::chrono::high_resolution_clock::time_point text_timeout_;
  std::shared_ptr<PPU> ppu;
  Bus* bus;

  void dump_fb(std::array<byte, BUF_WIDTH * BUF_HEIGHT> array);

  inline byte& at(size_t index) {
    return fb[index];
  }

  void set_paused(bool b);

  void frame_rendered(std::chrono::milliseconds ms);

  void toast(std::string text, std::chrono::milliseconds delay);

  SDL_Texture* make_raster_texture(size_t dx, size_t dy);
};
