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

  for (int row = 0; row < 1024; ++row) {
    for (int col = 0; col < 1024; ++col) {
      auto tile_col = col / 8;
      auto tile_row = row / 8;
      // look up tile map
      // tile_id = vram[tile_row * 128 + tile_col].h
      // tile_chr = vram[tile_id * 0x40 + (row % 8) * 8 + (col % 8)].l

//      auto tile_id = tile_row * 128 + tile_col;
////      std::printf("(%d, %d) -> %x (%d, %d)\n", row, col, tile_id,
////                  row % 8, col % 8);
//      auto addr = tile_id * 0x40 + (row % 8) * 8 + (col % 8);
//      auto pix = ppu->vram[addr].l;
      auto tile_id = ppu->vram[tile_row * 128 + tile_col].h;
      auto pix =  ppu->vram[tile_id * 0x40 + (row % 8) * 8 + (col % 8)].l;
//      if (pix != 0)
//      std::printf("(%d, %d) -> %d\n", row, col, pix);
      buf[row * 4 * 1024 + col * 4] = pix;
      buf[row * 4 * 1024 + col * 4 + 1] = pix;
      buf[row * 4 * 1024 + col * 4 + 2] = pix;
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
        case SDLK_TAB:
          auto* vram = ppu->vram.begin();
          for (int i = 0 ; i < 32; ++i) {
            printf("%x ", vram[i]);
          }
          printf("\n");

          break;
      }
    }
  }
}
