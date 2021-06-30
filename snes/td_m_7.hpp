//
// Created by Daniel Tse on 28/6/21.
//

#pragma once

#include "sppu.hpp"

constexpr size_t M7_WIDTH = 1024;
constexpr size_t M7_HEIGHT = 1024;

class M7 {
public:
  std::shared_ptr<SPPU> ppu;

  M7(): mode_(Mode::Viewport) {
    SDL_Init(SDL_INIT_VIDEO);

    window_ = SDL_CreateWindow("M7", 500, 0,
                               M7_WIDTH, M7_HEIGHT, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED
                                                | SDL_RENDERER_PRESENTVSYNC
                                                | SDL_RENDERER_TARGETTEXTURE);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, M7_WIDTH, M7_HEIGHT);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, M7_WIDTH, M7_HEIGHT);

    SDL_ShowWindow(window_);
  }

  ~M7() {
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
  }

  void connect(BusSNES* b);

  void show();

  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;

  enum class Mode {
    Viewport, Tiles
  };
  Mode mode_;

  std::array<byte, (M7_WIDTH * M7_HEIGHT) * 4> buf {};
};