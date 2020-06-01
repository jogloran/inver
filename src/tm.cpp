#include "tm.hpp"
#include "ppu.hpp"

void
TM::show() {
  SDL_ShowWindow(window_);
  // There are two sections, 0000-0fff and 1000-1fff, each containing 16*16=256 tiles
  // Each tile consists of 16 bytes, encoding palette values 0,1,2,3
  // The total pattern memory is 2*256*16 = 8192 bytes
  
  // Write 128*256 pixels
  auto* ptr = buf.data();
  for (int line = 0; line < 128; ++line) {
    for (int col = 0; col < 32; ++col) {
      byte row = line / 8;
      word tile_index = row * 16 + (col % 16);
      std::array<byte, 8> tile_row = ppu->decode(col / 16, tile_index, line % 8);
      for (int i = 0; i < 8; ++i) {
        *ptr++ = tile_row[i] * 64;
        *ptr++ = tile_row[i] * 64;
        *ptr++ = tile_row[i] * 64;
        *ptr++ = 255;
      }
    }
  }
  
  SDL_RenderPresent(renderer_);
  
  SDL_UpdateTexture(texture_, NULL, buf.data(), TM_WIDTH * 4);
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
