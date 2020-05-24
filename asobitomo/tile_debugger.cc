
#include <vector>

#include "tile_debugger.h"

#include "mmu.h"
#include "ppu_base.h"
#include "cpu.h"
#include "ppu_util.h"

void TD::show() {
  if (!enabled_) {
    return;
  }
  
  for (int i = 0; i < 256; ++i) {
    std::vector<PPU::PaletteIndex> tile_pixels;
    
    for (int m = 0; m < 8; ++m) {
      auto row = ppu_.tilemap_index_to_tile_debug(i, m);
      std::copy(row.begin(), row.end(), std::back_inserter(tile_pixels));
    }
    
    int row = i / 16;
    int col = i % 16;
    for (int k = 0; k < tile_pixels.size(); ++k) {
      int off_y = k / 8;
      int off_x = k % 8;
      
      int offset = (row*8 + off_y) * (TD_WIDTH * 4) + ((col*8 + off_x) * 4);
      buf[offset++] = tile_pixels[k] * 64;
      buf[offset++] = tile_pixels[k] * 64;
      buf[offset++] = tile_pixels[k] * 64;
      buf[offset++] = 255;
    }
  }
  
  SDL_UpdateTexture(texture_, NULL, buf.data(), TD_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);
  SDL_RenderPresent(renderer_);
}
