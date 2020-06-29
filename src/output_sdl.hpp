#pragma once

#include <array>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>
#include <gflags/gflags.h>
#include <chrono>

#include "nes_ntsc.h"
#include "types.h"
#include "renderer.hpp"
#include "output.hpp"

using namespace std::chrono_literals;

class PPU;

class Bus;

class SDLOutput : public Output {
public:
  SDLOutput(std::unique_ptr<Renderer> r);

  ~SDLOutput() {
    SDL_DestroyTexture(texture_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);

    TTF_CloseFont(font_);
    SDL_DestroyTexture(raster_);
  }

  void blit();

  static constexpr int BUF_WIDTH = 256;
  static constexpr int BUF_HEIGHT = 240;
  static constexpr int SCALE = 4;
  static constexpr int OUTPUT_WIDTH = BUF_WIDTH * SCALE;
  static constexpr int OUTPUT_HEIGHT = BUF_HEIGHT * SCALE;
  static constexpr std::chrono::milliseconds MS_PER_FRAME = 1000ms / 60;
  static constexpr std::chrono::milliseconds TOAST_FADE_OUT_TIME = 500ms;

  // Each byte of fb is a palette index (0...0x40)
  std::array<byte, BUF_WIDTH * BUF_HEIGHT * 4> buf;
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  TTF_Font* font_;
  std::string text_;
  SDL_Texture* text_texture_;
  SDL_Texture* raster_;
  std::chrono::high_resolution_clock::time_point text_timeout_;
  PPU* ppu;
  Bus* bus;
  std::unique_ptr<Renderer> renderer;

  void dump_fb(std::array<byte, BUF_WIDTH * BUF_HEIGHT> array);

  void set_paused(bool b) override;

  void frame_rendered(std::chrono::milliseconds ms) override;

  void toast(std::string text, std::chrono::milliseconds delay);

  SDL_Texture* make_raster_texture(size_t dx, size_t dy);

  std::shared_ptr<nes_ntsc_t> ntsc;
};
