#include "screen.hpp"
#include "sppu.hpp"

DECLARE_bool(show_raster);

void Screen::set_brightness(byte b) {
  // require: b >= 0 && b <= 15
  brightness = b;
}

void Screen::blit() {
  SDL_ShowWindow(window_);

  float scale = static_cast<float>(brightness) / 2.0;

  int i = 0;
//  for (auto& layer: fb) {
  auto& layer = fb[0];
  for (const colour_t& b: layer) {
    buf[i++] = b.b * scale;
    buf[i++] = b.g * scale;
    buf[i++] = b.r * scale;
    buf[i++] = 255;
  }

  auto ptr = (SPPU::OAM*) ppu->oam.data();
  for (int i = 0; i < 128; ++i) {
    SPPU::OAM oam = ptr[i];
    SPPU::OAM2* oam2 = (SPPU::OAM2*) ppu->oam.data() + 512 + (i / 4);
    word oam_x = compute_oam_x(oam, oam2, i);
    for (int j = oam.x; j < oam.x + 8; ++j) {
      buf[4 * (oam.y * SCREEN_WIDTH + j) + 2] = 255;
      buf[4 * ((oam.y + 7) * SCREEN_WIDTH + j) + 2] = 255;
    }
    for (int k = oam.y; k < oam.y + 8; ++k) {
      buf[4 * (k * SCREEN_WIDTH + oam.x) + 2] = 255;
      buf[4 * (k * SCREEN_WIDTH + oam.x + 7) + 2] = 255;
    }
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
          ppu->dump_bg(0);
          std::printf("\n");
          ppu->dump_bg(1);
          std::printf("\n");
          ppu->dump_bg(2);
          std::printf("\n");
          std::exit(0);
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

word Screen::compute_oam_x(SPPU::OAM oam, SPPU::OAM2* oam2, int i) {
  switch (i % 4) {
    case 0:
      return oam.x + (oam2->obj0_sz << 8);
    case 1:
      return oam.x + (oam2->obj1_sz << 8);
    case 2:
      return oam.x + (oam2->obj2_sz << 8);
    case 3:
      return oam.x + (oam2->obj3_sz << 8);
  }
}
