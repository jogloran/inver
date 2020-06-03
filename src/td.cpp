#include "td.hpp"
#include "ppu.hpp"

void
TD::show() {
  SDL_ShowWindow(window_);
  // There are two sections, 0000-0fff and 1000-1fff, each containing 16*16=256 tiles
  // Each tile consists of 16 bytes, encoding palette values 0,1,2,3
  // The total pattern memory is 2*256*16 = 8192 bytes
  
  // Write 128*256 pixels
  auto* ptr = buf.data();

  for (int half = 0; half < 2; ++half) {
    for (int line = 0; line < 30 * 8; ++line) {
      for (int col = 0; col < 64; ++col) {
        word base = 0x2000;
        byte plane = 2 * half + (col < 32);
        base |= plane << 10;
        byte nt_byte = ppu->ppu_read(base + 32 * (line / 8) + (col % 32));
//        byte at_byte = ppu->ppu_read(0x23c0 | (plane << 10) + 8 * (line / 8) + (col % 32));
        byte lsb = ppu->ppu_read((ppu->ppuctrl.background_pattern_address << 12) + (nt_byte << 4) + (line % 8));
        byte msb = ppu->ppu_read((ppu->ppuctrl.background_pattern_address << 12) + (nt_byte << 4) + 8 + (line % 8));
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
  
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
  SDL_Event event;
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      std::exit(0);
    }
  }
}
