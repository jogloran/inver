#include "screen.hpp"
#include "ppu.hpp"
#include "bus.hpp"

#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

DECLARE_bool(show_raster);
DECLARE_bool(fake_sprites);

struct rgb {
  byte r;
  byte g;
  byte b;
};

std::array<rgb, 0x40> ntsc_palette {
    0x59, 0x59, 0x59, // 0x00
    0x00, 0x09, 0x89, // 0x01
    0x17, 0x00, 0x8a, // 0x02
    0x37, 0x00, 0x6e, // 0x03
    0x54, 0x00, 0x51, // 0x04
    0x54, 0x00, 0x0e, // 0x05
    0x54, 0x0a, 0x00, // 0x06
    0x3b, 0x17, 0x00, // 0x07
    0x22, 0x26, 0x00, // 0x08
    0x0a, 0x2a, 0x00, // 0x09
    0x00, 0x2b, 0x00, // 0x0a
    0x00, 0x29, 0x27, // 0x0b
    0x00, 0x22, 0x59, // 0x0c
    0x00, 0x00, 0x00, // 0x0d
    0x08, 0x08, 0x08, // 0x0e
    0x08, 0x08, 0x08, // 0x0f
    0xa6, 0xa6, 0xa6, // 0x10
    0x00, 0x3b, 0xc5, // 0x11
    0x47, 0x25, 0xf6, // 0x12
    0x6c, 0x00, 0xe1, // 0x13
    0x95, 0x0a, 0xae, // 0x14
    0x9e, 0x0e, 0x4d, // 0x15
    0x8c, 0x28, 0x00, // 0x16
    0x7a, 0x41, 0x00, // 0x17
    0x59, 0x50, 0x00, // 0x18
    0x23, 0x57, 0x00, // 0x19
    0x00, 0x5e, 0x00, // 0x1a
    0x00, 0x5e, 0x44, // 0x1b
    0x00, 0x53, 0x87, // 0x1c
    0x08, 0x08, 0x08, // 0x1d
    0x08, 0x08, 0x08, // 0x1e
    0x08, 0x08, 0x08, // 0x1f
    0xe6, 0xe6, 0xe6, // 0x20
    0x4c, 0x88, 0xff, // 0x21
    0x70, 0x75, 0xff, // 0x22
    0x90, 0x5c, 0xff, // 0x23
    0xb4, 0x5a, 0xe1, // 0x24
    0xc7, 0x5a, 0x99, // 0x25
    0xd4, 0x6d, 0x48, // 0x26
    0xc7, 0x83, 0x06, // 0x27
    0xae, 0x9c, 0x00, // 0x28
    0x6c, 0xa6, 0x00, // 0x29
    0x2e, 0xab, 0x2e, // 0x2a
    0x28, 0xb0, 0x7a, // 0x2b
    0x1f, 0xaf, 0xcc, // 0x2c
    0x40, 0x40, 0x40, // 0x2d
    0x08, 0x08, 0x08, // 0x2e
    0x08, 0x08, 0x08, // 0x2f
    0xe6, 0xe6, 0xe6, // 0x30
    0xa2, 0xc3, 0xf3, // 0x31
    0xad, 0xad, 0xf8, // 0x32
    0xb7, 0xa2, 0xf3, // 0x33
    0xcc, 0xa8, 0xe1, // 0x34
    0xd9, 0xa9, 0xd0, // 0x35
    0xd9, 0xae, 0xa3, // 0x36
    0xd9, 0xbb, 0x91, // 0x37
    0xd9, 0xd0, 0x8d, // 0x38
    0xbf, 0xd7, 0x90, // 0x39
    0xae, 0xd9, 0xa5, // 0x3a
    0xa1, 0xd9, 0xbe, // 0x3b
    0xa1, 0xcf, 0xd9, // 0x3c
    0xab, 0xab, 0xab, // 0x3d
    0x08, 0x08, 0x08, // 0x3e
    0x08, 0x08, 0x08, // 0x3f
};

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

  for (int x = 0; x < BUF_WIDTH * SCALE; x += dx * SCALE / 2) {
    SDL_RenderDrawLine(renderer_, x, 0, x, BUF_HEIGHT * SCALE);
  }

  for (int y = 0; y < BUF_HEIGHT * SCALE; y += dy * SCALE / 2) {
    SDL_RenderDrawLine(renderer_, 0, y, BUF_WIDTH * SCALE, y);
  }

  SDL_SetRenderTarget(renderer_, nullptr);

  return raster;
}

void
Screen::blit() {
  int i = 0;

  int pitch;
  unsigned char* pixels;
  SDL_LockTexture(texture_, nullptr, (void**) &pixels, &pitch);

  for (byte b: fb) {
    pixels[i++] = ntsc_palette[b].b;
    pixels[i++] = ntsc_palette[b].g;
    pixels[i++] = ntsc_palette[b].r;
    pixels[i++] = 255;
  }

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

//  SDL_UpdateTexture(texture_, NULL, buf.data(), Screen::BUF_WIDTH * 4);
//  SDL_RenderClear(renderer_);

//  std::cout << pitch << std::endl;
//  std::copy(buf.begin(), buf.end(), pixels);
  SDL_UnlockTexture(texture_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);

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
