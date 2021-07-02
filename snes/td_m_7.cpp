//
// Created by Daniel Tse on 28/6/21.
//

#include "td_m_7.hpp"

#include "bus_snes.hpp"

void M7::connect(BusSNES* b) {
  ppu = b->ppu;
}

void M7::show() {
  SDL_SetRenderDrawColor(renderer_, 0, 127, 0, 255);
//
//  for (int i = 0; i < 8; ++i) {
//      for (int j = 0; j < 8; ++j) {
//          std::printf("%02x ", ppu->vram[0x40 * 1 + i*8+j].h);
//      }
//      std::printf("\n");
//  }
//  std::printf("\n");

  for (int row = 0; row < 1024; ++row) {
    for (int col = 0; col < 1024; ++col) {
      auto tile_col = col / 8;
      auto tile_row = row / 8;

      byte tile_id;
      if (mode_ == Mode::Tiles) {
        // Trying to show tiles
        tile_id = tile_row * 128 + tile_col;
      } else {
        // Trying to deref tilemap
        tile_id = ppu->vram[tile_row * 128 + tile_col].m7_tile_id;
      }
      auto pix = ppu->vram[tile_id * 0x40 + (row % 8) * 8 + (col % 8)].m7_chr;

      auto colour = ppu->lookup(pix);
      buf[row * 4 * 1024 + col * 4] = colour.b * 4;
      buf[row * 4 * 1024 + col * 4 + 1] = colour.g * 4;
      buf[row * 4 * 1024 + col * 4 + 2] = colour.r * 4;
      buf[row * 4 * 1024 + col * 4 + 3] = 255;
    }
  }

  SDL_UpdateTexture(texture_, nullptr, buf.data(), M7_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);

  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

  SDL_RenderPresent(renderer_);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      std::exit(0);
    } else if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
        case SDLK_TAB: {
          if (mode_ == Mode::Viewport) { mode_ = Mode::Tiles; }
          else { mode_ = Mode::Viewport; }
          break;
        }
      }
    }
  }
}
