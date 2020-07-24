#include "screen.hpp"
#include "sppu.hpp"

DECLARE_bool(show_raster);

void Screen::blit() {
  SDL_ShowWindow(window_);

  int i = 0;
//  for (auto& layer: fb) {
  auto& layer = fb[0];
  for (const colour_t& b: layer) {
    buf[i++] = b.b * 8;
    buf[i++] = b.g * 8;
    buf[i++] = b.r * 8;
    buf[i++] = 255;
  }
  i = 0;
  layer = fb[1];
  for (const colour_t& b: layer) {
    if (buf[i] == 0)
      buf[i++] = b.b * 8;
    if (buf[i] == 0)
    buf[i++] = b.g * 8;
    if (buf[i] == 0)
    buf[i++] = b.r * 8;
    buf[i++] = 255;
  }
//  i = 0;
//  layer = fb[2];
//  for (const colour_t& b: layer) {
//    buf[i++] = b.b * 8;
//    buf[i++] = b.g * 8;
//    buf[i++] = b.r * 8;
//    buf[i++] = 255;
//  }
//  }

  SDL_UpdateTexture(texture_, nullptr, buf.data(), SCREEN_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, nullptr, nullptr);

  if (FLAGS_show_raster) {
    raster_ = make_raster_texture(16, 16);
    SDL_RenderCopy(renderer_, raster_, nullptr, nullptr);
    SDL_DestroyTexture(raster_);
  }

  SDL_RenderPresent(renderer_);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      std::exit(0);
    } else if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
        case SDLK_d:
          ppu->dump_bg();
          break;
        case SDLK_p:
          ppu->dump_pal();
          break;
        case SDLK_s:
          ppu->dump_sprite();
          break;
      }
    }
  }
}

void Screen::connect(SPPU* p) {
  ppu = p;
}
