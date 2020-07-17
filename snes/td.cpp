#include "td.hpp"
#include "sppu.hpp"
#include "bus_snes.hpp"

#include <SDL_ttf.h>

void TD2::draw_text(const char* txt, SDL_Rect rect) {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_Color fg {255, 255, 255, 255};
  SDL_Surface* text_surface = TTF_RenderText_Blended(font_, txt, fg);

  SDL_Texture* text_texture_ = SDL_CreateTextureFromSurface(renderer_, text_surface);
  SDL_RenderCopy(renderer_, text_texture_, nullptr, &rect);
  SDL_DestroyTexture(text_texture_);
  SDL_FreeSurface(text_surface);
}

void
TD2::show() {
  SDL_ShowWindow(window_);

  SDL_RenderPresent(renderer_);

  SDL_UpdateTexture(texture_, nullptr, buf.data(), TD2_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);

  static char buf[128] = {};
  // 512 indices
  SDL_SetRenderDrawColor(renderer_, 0, 127, 0, 255);
  dword base = ppu->bg_base_size[0].base_addr * 0x400;
  for (dword i = base; i < base + 32 * 32; ++i) {
    SPPU::bg_map_tile_t* t = (SPPU::bg_map_tile_t*) &ppu->vram[i];

    int offset = i - base;
    SDL_Rect rect {(offset % 32) * 32, (offset / 32) * 32, 16, 32};
    sprintf(buf, "%02x", t->char_no);
    draw_text(buf, rect);
  }

  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      std::exit(0);
    }
  }
}

void TD2::connect(BusSNES* b) {
  ppu = &b->ppu;
}

