#pragma once

#include <array>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <gflags/gflags.h>
#include <chrono>

#include "types.h"

class PPU;

class Bus;

class Screen {
public:
  Screen() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    TTF_Init();
    window_ = SDL_CreateWindow("Game",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               BUF_WIDTH * 4, BUF_HEIGHT * 4, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1,
                                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, BUF_WIDTH * 4, BUF_HEIGHT * 4);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 BUF_WIDTH, BUF_HEIGHT);
    font_ = TTF_OpenFont("/Library/Fonts/Futura.ttc", 24); //this opens a font style and sets a size
//    TTF_SetFontOutline(font_, 2);

    SDL_ShowWindow(window_);

    std::fill(fb.begin(), fb.end(), 0);
    std::fill(buf.begin(), buf.end(), 0);
  }

  void blit();

  static constexpr int BUF_WIDTH = 256;
  static constexpr int BUF_HEIGHT = 240;
  static constexpr int MS_PER_FRAME = 1000 / 60;

  // Each byte of fb is a palette index (0...0x40)
  std::array<byte, BUF_WIDTH * BUF_HEIGHT> fb;
  std::array<byte, BUF_WIDTH * BUF_HEIGHT * 4> buf;
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  TTF_Font* font_;
  std::string text_;
  SDL_Texture* text_texture_;
  std::chrono::high_resolution_clock::time_point text_timeout_;
  std::shared_ptr<PPU> ppu;
  Bus* bus;

  void dump_fb(std::array<byte, BUF_WIDTH * BUF_HEIGHT> array);

  inline byte& at(size_t index) {
    return fb[index];
  }

  void set_paused(bool b);

  void frame_rendered(uint32_t ms);

  void toast(std::string text, std::chrono::milliseconds delay);
};
