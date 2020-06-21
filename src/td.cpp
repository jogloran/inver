#include "td.hpp"
#include "ppu.hpp"

void
TD::show() {
  SDL_ShowWindow(window_);

  auto* ptr = buf.data();

  for (int half = 0; half < 2; ++half) {
    for (int line = 0; line < 30 * 8; ++line) {
      for (int col = 0; col < 64; ++col) {
        word base = 0x2000;
        byte plane = 2 * half + (col < 32);
        base |= plane << 10;
        byte nt_byte = ppu->ppu_read(base + 32 * (line / 8) + (col % 32));

        word lsb_addr =
            (ppu->ppuctrl.bg_pt_addr << 12) + (nt_byte << 4) + (line % 8);
        byte lsb = ppu->ppu_read(lsb_addr);
        byte msb = ppu->ppu_read(lsb_addr + 8);
        std::array<byte, 8> tile_row = unpack_bits(lsb, msb);

        for (int i = 0; i < 8; ++i) {
          *ptr++ = tile_row[i] * 64;
          *ptr++ = tile_row[i] * 64;
          *ptr++ = tile_row[i] * 64;
          *ptr++ = 255;
        }
      }
    }
  }

  SDL_RenderPresent(renderer_);

  SDL_UpdateTexture(texture_, NULL, buf.data(), TD_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);

  SDL_Rect rect {
      .x = (8*ppu->loopy_t.coarse_x + ppu->fine_x) * 2,
      .y = (8*ppu->loopy_t.coarse_y + ppu->loopy_t.fine_y) * 2,
      .w = TD_WIDTH,
      .h = TD_HEIGHT,
  };
  SDL_SetRenderDrawColor(renderer_, 0x09, 0x84, 0xe3, 80);
  SDL_RenderFillRect(renderer_, &rect);
  SDL_SetRenderDrawColor(renderer_, 0x09, 0x84, 0xe3, 255);
  SDL_RenderDrawRect(renderer_, &rect);

  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
}

void TD::connect(Bus* b) {
  ppu = b->ppu;
}
