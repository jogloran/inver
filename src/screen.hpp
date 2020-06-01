#pragma once

#include <array>
#include <SDL2/SDL.h>
#include <gflags/gflags.h>

#include "types.h"

class PPU;

class Bus;

class Screen {
public:
  Screen() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
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

    SDL_ShowWindow(window_);

    std::fill(fb.begin(), fb.end(), 0);
    std::fill(buf.begin(), buf.end(), 0);
  }

  void blit();

  template<typename T>
  void set_row(int row, T begin, T end) {
    std::copy_n(begin, BUF_WIDTH, fb.begin() + row * BUF_WIDTH);
  }

  static constexpr int BUF_WIDTH = 256;
  static constexpr int BUF_HEIGHT = 240;

  // Each byte of fb is a palette index (0...0x40)
  std::array<byte, BUF_WIDTH * BUF_HEIGHT> fb;
  std::array<byte, BUF_WIDTH * BUF_HEIGHT * 4> buf;

  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;

  PPU* ppu;
  Bus* bus;

  void dump_fb(std::array<byte, BUF_WIDTH * BUF_HEIGHT> array);

  void frame_rendered(double ms);

  inline byte& at(size_t index) {
    return fb[index];
  }
};
