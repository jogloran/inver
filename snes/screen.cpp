#include "screen.hpp"

void Screen::blit() {
  SDL_ShowWindow(window_);

  int i = 0;
//  for (auto& layer: fb) {
  auto& layer = fb[0];
  for (const colour_t& b: layer) {
    buf[i++] = b.r > 0 ? 255 : 0;
    buf[i++] = b.g > 0 ? 255 : 0;
    buf[i++] = b.b > 0 ? 255 : 0;
    buf[i++] = 255;
  }
//  }

  SDL_UpdateTexture(texture_, nullptr, buf.data(), SCREEN_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);
  SDL_RenderPresent(renderer_);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      std::exit(0);
    }
  }
}

void Screen::connect(SPPU* p) {
  ppu = p;
}
