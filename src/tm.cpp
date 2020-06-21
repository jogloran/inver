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
//      std::printf("(%1.1d, %3.3d, %3.3d) reading from %04x\n", col/16, row, col % 16,
//                  ((col/16) << 12) + tile_index * 16 + (line % 8));
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

  SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);
  SDL_RenderDrawLine(renderer_, x, 0, x, y - 10);
  SDL_RenderDrawLine(renderer_, x, y + 10, x, TM_HEIGHT * 4);
  SDL_RenderDrawLine(renderer_, 0, y, x - 10, y);
  SDL_RenderDrawLine(renderer_, x + 10, y, TM_WIDTH * 4, y);

  auto pos_x = x / 4, pos_y = y / 4;
  auto col = pos_x / 8;
  auto row = pos_y / 8;
  SDL_Rect rect = {
      col * 4 * 8, row * 4 * 8, 8 * 4, 8 * 4
  };
  SDL_RenderDrawRect(renderer_, &rect);

  selected_tile = row * 16 + (col % 16);
  if (col >= 16) selected_tile += 256;
  render_tooltip(selected_tile);
  
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

  if (SDL_GetWindowID(SDL_GetMouseFocus()) == SDL_GetWindowID(window_)) {
    auto state = SDL_GetMouseState(&x, &y);
    if (state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
      cl_x = x;
      cl_y = y;
    }
  }
}

void TM::connect(Bus* b) {
  ppu = b->ppu;
}
