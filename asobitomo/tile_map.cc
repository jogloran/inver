#include "tile_map.h"

#include "ppu_base.h"
#include "cpu.h"
#include "ppu_util.h"

void
TM::show() {
  if (!enabled_) {
    return;
  }
  
  word tilemap_offset = ppu_.bg_tilemap_offset;
  
  for (int row = 0; row < 32; ++row) {
    for (int col = 0; col < 32; ++col) {
      int idx = row * 32 + col;
      word item = tilemap_offset + idx; // This is the address of the tile index
      byte tile_index = ppu_.cpu.mmu[item];
      
      // Get the tile data offset
      word tile_data_address;
      if (ppu_.bg_window_tile_data_offset == 0x8000) {
        tile_data_address = ppu_.bg_window_tile_data_offset + tile_index * 16;
      } else {
        tile_data_address = ppu_.bg_window_tile_data_offset + 0x800 + (static_cast<signed char>(tile_index))*16;
      }
      
      // Decode tile
      std::vector<PPU::PaletteIndex> tile_pixels;
      for (int m = 0; m < 8; ++m) {
        auto row = ppu_.tilemap_index_to_tile_debug(tile_index, m);
        std::copy(row.begin(), row.end(), std::back_inserter(tile_pixels));
      }
      
      // Place tile at location
      for (int k = 0; k < tile_pixels.size(); ++k) {
        int off_y = k / 8;
        int off_x = k % 8;
        
        int offset = (row*8 + off_y) * (TM_WIDTH * 4) + ((col*8 + off_x) * 4);
      
        buf[offset++] = tile_pixels[k] * 64;
        buf[offset++] = tile_pixels[k] * 64;
        buf[offset++] = tile_pixels[k] * 64;
        buf[offset++] = 255;
      }
    }
  }
  
  byte scx = ppu_.cpu.mmu[0xff43];
  byte scy = ppu_.cpu.mmu[0xff42];
  
//  static char s[64];
//  sprintf(s, "scx %02x scy %02x", scx, scy);
//  SDL_SetWindowTitle(window_, s);
  
  SDL_UpdateTexture(texture_, NULL, buf.data(), TM_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);
  
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
  
  int alpha = std::min(256 - scx, 160) * scale_;
  int beta = std::min(256 - scy, 144) * scale_;
  int gamma = (160 - alpha) * scale_;
  int delta = (144 - beta) * scale_;
  
  SDL_Rect rects[] {
    { scx*scale_, scy*scale_, alpha, beta },
    { 0, scy*scale_, gamma, beta},
    { scx*scale_, 0, alpha, delta},
    { 0, 0, gamma, delta }
  };

  SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 64);
  for (SDL_Rect rect : rects) {
    SDL_RenderFillRect(renderer_, &rect);
  }
  
  SDL_SetRenderDrawColor(renderer_, 244, 177, 18, 64);
  for (int i = 0; i < 256; i += 8) {
    SDL_RenderDrawLine(renderer_, 0, i*scale_, TM_HEIGHT*scale_, i*scale_);
    SDL_RenderDrawLine(renderer_, i*scale_, 0, i*scale_, TM_WIDTH*scale_);
  }
  
  SDL_RenderPresent(renderer_);
  
  int mouse_x, mouse_y;
  uint8_t mouse_mask = SDL_GetMouseState(&mouse_x, &mouse_y);
  if (mouse_mask & SDL_BUTTON(1)) {
    mouse_x /= 8;
    mouse_y /= 8;

    std::cout << "address: " << setw(4) << setfill('0') << hex <<  (ppu_.bg_tilemap_offset + (mouse_y / scale_) * 32 + (mouse_x / scale_)) << std::endl;
    byte tile_index = ppu_.cpu.mmu[ppu_.bg_tilemap_offset + (mouse_y / scale_) * 32 + (mouse_x / scale_)];
    std::cout << "tile data offset: " << hex << setw(4) << ppu_.bg_window_tile_data_offset << std::endl;
    std::cout << "tile data start: " << hex << setw(4) << (ppu_.bg_window_tile_data_offset + 0x800 + (static_cast<signed char>(tile_index))*16) << std::endl;

    for (int i = 0; i < 8; ++i) {
      auto row = ppu_.tilemap_index_to_tile_debug(tile_index, i);

      for (PPU::PaletteIndex idx: row) {
        word addr = ppu_.bg_window_tile_data_offset + 0x800 +
          static_cast<signed char>(tile_index)*16 + i*2;
        std::cout << setw(4) << addr << ' ' << Console::d[idx] << Console::d[idx] << ' ';
      }
      std::cout << std::endl;
    }

     std::cout << hex << setfill('0') << setw(2) << int(mouse_x / scale_) << ' ' << setw(2) << int(mouse_y / scale_) << ' ' << setw(2) << int(tile_index) << std::endl;
  }
}
