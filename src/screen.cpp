#include "screen.hpp"
#include "ppu.hpp"
#include "nes_ntsc.h"
#include "renderer.hpp"
#include "renderer_ntsc.hpp"
#include "renderer_palette.hpp"

#include <SDL2/SDL.h>
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

DECLARE_bool(show_raster);
DECLARE_bool(fake_sprites);
DECLARE_bool(kb);


void
Screen::toast(std::string text, std::chrono::milliseconds delay) {
  text_ = text;
  text_timeout_ = std::chrono::high_resolution_clock::now() + delay;
}

SDL_Texture* Screen::make_raster_texture(size_t dx, size_t dy) {
  SDL_Texture* raster = SDL_CreateTexture(renderer_, SDL_GetWindowPixelFormat(window_),
                                          SDL_TEXTUREACCESS_TARGET,
                                          BUF_WIDTH * SCALE, BUF_HEIGHT * SCALE);
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

void
Screen::blit() {
  int pitch;
  unsigned char* pixels;
  SDL_LockTexture(texture_, nullptr, (void**) &pixels, &pitch);

  renderer->render(fb, BUF_WIDTH, pixels, pitch, BUF_HEIGHT);

  SDL_UnlockTexture(texture_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);

  if (FLAGS_fake_sprites) {
    for (const PPU::OAM& oam : ppu->oam) {
      for (int j = oam.x; j < oam.x + 8; ++j) {
        buf[4 * (oam.y * BUF_WIDTH + j) + 2] = 255;
        buf[4 * ((oam.y + 7) * BUF_WIDTH + j) + 2] = 255;
      }
      for (int k = oam.y; k < oam.y + 8; ++k) {
        buf[4 * (k * BUF_WIDTH + oam.x) + 2] = 255;
        buf[4 * (k * BUF_WIDTH + oam.x + 7) + 2] = 255;
      }
    }
  }

  if (text_.size()) {
    auto now {std::chrono::high_resolution_clock::now()};
    if (now <= text_timeout_) {
      uint8_t alpha = (text_timeout_ - now > TOAST_FADE_OUT_TIME)
                      ? 255
                      : 255 * (text_timeout_ - now) / TOAST_FADE_OUT_TIME;
      SDL_Surface* text_surface = TTF_RenderText_Blended(font_, text_.c_str(),
                                                         {255, 255, 255, alpha});
      text_texture_ = SDL_CreateTextureFromSurface(renderer_, text_surface);
      SDL_Rect rect = {16, 16, text_surface->w, text_surface->h};
      SDL_RenderCopy(renderer_, text_texture_, nullptr, &rect);
      SDL_DestroyTexture(text_texture_);
      SDL_FreeSurface(text_surface);
    }
  }

  if (FLAGS_show_raster) {
    raster_ = make_raster_texture(16, 16);
    SDL_RenderCopy(renderer_, raster_, nullptr, nullptr);
    SDL_DestroyTexture(raster_);
  }

  SDL_RenderPresent(renderer_);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      std::exit(0);
    } else if (event.type == SDL_KEYDOWN) {
      // If Family Basic keyboard is connected, don't consume key events
      if (FLAGS_kb) continue;

      switch (event.key.keysym.sym) {
        case SDLK_d:
          ppu->dump_nt();
          break;
        case SDLK_p:
          ppu->dump_pt();
          break;
        case SDLK_a:
          ppu->dump_at();
          break;
        case SDLK_o:
          dump_fb(fb);
          break;
        case SDLK_s:
          ppu->dump_oam();
          break;
        case SDLK_r:
          bus->reset();
          break;
        case SDLK_ESCAPE:
          bus->toggle_pause();
          break;
        case SDLK_w:
          bus->request_save();
          break;
        case SDLK_m:
          bus->cart->dump_mapper();
          std::exit(0);
          break;
        case SDLK_v:
          toast("Capturing savestate", 1s);
          bus->pickle("save.state");
          break;
        case SDLK_e:
          bus->unpickle("save.state");
          toast("Loading savestate", 1s);
          break;
        case SDLK_PERIOD:
          FLAGS_show_raster = !FLAGS_show_raster;
          break;
        case SDLK_q:
          std::exit(0);
      }
    }
  }
}

void Screen::dump_fb(std::array<byte, BUF_WIDTH * BUF_HEIGHT> sc) {
  for (int row = 0; row < BUF_WIDTH; ++row) {
    for (int col = 0; col < BUF_HEIGHT; ++col) {
      std::printf("%02x", sc[row * BUF_WIDTH + col]);
    }
    std::printf("\n");
  }
}

void Screen::frame_rendered(std::chrono::milliseconds ms) {
  auto then = std::chrono::high_resolution_clock::now();
  blit();
  auto now = std::chrono::high_resolution_clock::now();

  auto frame_time = ms +
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - then);
//  SDL_Delay((MS_PER_FRAME - std::min<>(MS_PER_FRAME, frame_time)).count());
}

void Screen::set_paused(bool paused) {
  if (paused) {
    SDL_SetTextureColorMod(texture_, 192, 192, 192);
    toast("Paused", 1000ms);
  } else {
    SDL_SetTextureColorMod(texture_, 255, 255, 255);
    toast("Unpaused", 1000ms);
  }
}

Screen::Screen() : renderer(std::make_unique<NTSCRenderer>()) {
  renderer->init();

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

  auto [texture_w, texture_h] = renderer->get_output_size();
  auto pixel_format = renderer->get_pixel_format();
  texture_ = SDL_CreateTexture(renderer_,
                               pixel_format == Renderer::PixelFormat::ARGB8888
                               ? SDL_PIXELFORMAT_ARGB8888 : SDL_PIXELFORMAT_RGB565,
                               SDL_TEXTUREACCESS_STREAMING,
                               texture_w, texture_h);

  SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
  font_ = TTF_OpenFont("mplus-2c-medium.ttf", 24);
//    TTF_SetFontOutline(font_, 2);
  raster_ = make_raster_texture(8, 8);

  SDL_ShowWindow(window_);

  std::fill(fb.begin(), fb.end(), 0);
  std::fill(buf.begin(), buf.end(), 0);
}

