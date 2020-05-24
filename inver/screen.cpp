#include "screen.hpp"
#include "ppu.hpp"

#include <iostream>

DECLARE_bool(show_raster);

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
Screen::blit() {
//  auto then = std::chrono::high_resolution_clock::now();
  int i = 0;
  for (byte b: fb) {
    bool show_vert = i % 32 == 0;
    bool show_horz = (i / 1024) % 8 == 0;
    bool draw_raster = FLAGS_show_raster && (show_vert || show_horz);
    buf[i++] = ntsc_palette[b].b;
    buf[i++] = (draw_raster) ? 255 : ntsc_palette[b].g;
    buf[i++] = ntsc_palette[b].r;
    buf[i++] = 255;
  }

  SDL_UpdateTexture(texture_, NULL, buf.data(), Screen::BUF_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);
  SDL_RenderPresent(renderer_);

//  SDL_PumpEvents();

  int nkeys;
  const uint8_t *keystates = SDL_GetKeyboardState(&nkeys);
  if (keystates[SDL_SCANCODE_D]) {
    ppu->dump_nt();
  } else if (keystates[SDL_SCANCODE_P]) {
    ppu->dump_pt();
  } else if (keystates[SDL_SCANCODE_A]) {
    ppu->dump_at();
  } else if (keystates[SDL_SCANCODE_O]) {
    dump_fb(fb);
  }

  SDL_Event event;
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      std::exit(0);
    }
  }

//  auto now = std::chrono::high_resolution_clock::now();
//  std::cout << "tick: "
//            << std::chrono::duration_cast<std::chrono::duration<double>>(now - then).count()
//            << std::endl;
}

void Screen::dump_fb(std::array<byte, BUF_WIDTH * BUF_HEIGHT> sc) {
  for (int row = 0; row < BUF_WIDTH; ++row) {
    for (int col = 0; col < BUF_HEIGHT; ++col) {
      std::printf("%02x", sc[row * BUF_WIDTH + col]);
    }
    std::printf("\n");
  }
}
