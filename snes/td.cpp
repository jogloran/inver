#include "td.hpp"
#include "sppu.hpp"
#include "bus_snes.hpp"

#include <SDL_ttf.h>
#include "ppu_utils.hpp"

void TD2::draw_text(const char* txt, SDL_Rect rect) {
  SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
  SDL_Color fg {255, 255, 255, 255};
  SDL_Surface* text_surface = TTF_RenderText_Blended(font_, txt, fg);

  SDL_Texture* text_texture_ = SDL_CreateTextureFromSurface(renderer_, text_surface);
  SDL_RenderCopy(renderer_, text_texture_, nullptr, &rect);
  SDL_DestroyTexture(text_texture_);
  SDL_FreeSurface(text_surface);
}

static constexpr byte wpp_for_bg[] = {2, 2, 1};

void
TD2::show() {
//  static char buf[128] = {};
  // 512 indices
  SDL_SetRenderDrawColor(renderer_, 0, 127, 0, 255);
  dword base = ppu->bg_base_size[bg].base_addr * 0x400;
//  word obj_char_data_base = ppu->obsel.obj_base_addr * 8192;
  dword chr_base_addr = ppu->bg_chr_base_addr_for_bg(bg);
  const byte wpp = wpp_for_bg[bg];

  std::vector<byte> sprite;
  sprite.reserve(64);
  for (int row = 0; row < 64; ++row) {
    for (int col = 0; col < 64; ++col) {
      word ad = addr(base, col, row, col >= 32, row >= 32);
      bg_map_tile_t* t = (bg_map_tile_t*) &ppu->vram[ad];
      sprite.clear();
      auto inserter = std::back_inserter(sprite);
      for (int i = 0; i < 8; ++i) {
        if (t->flip_y) {
          i = 7 - i;
        }
        word tile_chr_base = chr_base_addr + (8 * wpp) * t->char_no + i;
        auto tile_row = decode_planar(&ppu->vram[tile_chr_base], wpp * 2, t->flip_x);
        std::copy(tile_row.begin(), tile_row.end(), inserter);
      }

      int i = 0;
      // pitch (number of bytes per row) is 8 * 64 * 4
      for (byte b : sprite) {
        buf[(8 * 64 * 4 * (8 * row + i / 8)) + (8 * col + i % 8) * 4 ] = b * 64;
        buf[(8 * 64 * 4 * (8 * row + i / 8)) + (8 * col + i % 8) * 4  + 1] = b * 64;
        buf[(8 * 64 * 4 * (8 * row + i / 8)) + (8 * col + i % 8) * 4  + 2] = b * 64;
        buf[(8 * 64 * 4 * (8 * row + i / 8)) + (8 * col + i % 8) * 4  + 3] = 255;
        ++i;
      }
//      SDL_UpdateTexture(sprite_texture, nullptr, sprite.data(), 8);
//      auto h = std::hash<int> {}(t->char_no);
//      SDL_SetRenderDrawColor(renderer_, h & 0xff, (h >> 8) & 0xff, h >> 16, 255);
//      SDL_RenderFillRect(renderer_, &rect);
//      SDL_RenderCopy(renderer_, sprite_texture, nullptr, &rect);
//    draw_text(buf, rect);
    }
  }

  SDL_UpdateTexture(texture_, nullptr, buf.data(), TD2_WIDTH * 4);
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
          bg = (bg + 1) % 3;
          std::printf("%d\n", bg);
          break;
      }
    }
  }
}

void TD2::connect(BusSNES* b) {
  ppu = &b->ppu;
}

