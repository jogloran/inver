#pragma once

#include "types.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <gflags/gflags.h>
#include <array>

class PPU;

class TM {
public:
  PPU* ppu;
  TM() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window_ = SDL_CreateWindow("TM", 500, 0, TM_WIDTH * 4, TM_HEIGHT * 4, SDL_WINDOW_HIDDEN);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TM_WIDTH * 4, TM_HEIGHT * 4);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TM_WIDTH, TM_HEIGHT);

    font_ = TTF_OpenFont("mplus-2c-medium.ttf", 14);
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
  TTF_Font* font_;

  int32_t x, y;
  int32_t cl_x, cl_y;

  int selected_tile;

  void render_tooltip(int tile) {
    SDL_Rect rect {
      x + 15, y + 15, 50, 50
    };
    SDL_SetRenderDrawColor(renderer_, 64, 144, 173, 240);
//    SDL_RenderDrawRect(renderer_, &rect);
    SDL_RenderFillRect(renderer_, &rect);

    static char buf[32] = { 0 };
    std::sprintf(buf, "%04x", tile << 4);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_Color fg { 0, 0, 0, 255 };
    SDL_Surface* text_surface = TTF_RenderText_Blended(font_, buf, fg);

    SDL_Texture* text_texture_ = SDL_CreateTextureFromSurface(renderer_, text_surface);
    SDL_Rect text_rect = {x + 18, y + 18, text_surface->w, text_surface->h};
    SDL_RenderCopy(renderer_, text_texture_, nullptr, &text_rect);
    SDL_DestroyTexture(text_texture_);
    SDL_FreeSurface(text_surface);
  }
};
