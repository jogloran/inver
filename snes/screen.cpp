#include "screen.hpp"
#include "sppu.hpp"
#include "bus_snes.hpp"
#include "ppu_utils.hpp"
#include "ppu_debug.hpp"

DECLARE_bool(show_raster);
DECLARE_bool(dis);
DECLARE_bool(fake_sprites);
DECLARE_bool(show_main);
DECLARE_bool(show_sub);

void Screen::set_brightness(byte b) {
  // require: b >= 0 && b <= 15
  brightness = b;
}

void Screen::blit() {
  SDL_ShowWindow(window_);

  float scale = static_cast<float>(brightness) / 2.0;

  int i = 0;
//  for (auto& layer: fb) {
  auto &layer = fb;
  for (const colour_t &b: layer) {
    buf[i++] = b.b * scale;
    buf[i++] = b.g * scale;
    buf[i++] = b.r * scale;
    buf[i++] = 255;
  }

  if (FLAGS_fake_sprites) {
    auto ptr = (OAM*) ppu->oam.data();
    for (int i = 0; i < 128; ++i) {
      OAM* oam = &ptr[i];
      OAM2* oam2 = (OAM2*) ppu->oam.data() + 512 + (i / 4);
      word oam_x = compute_oam_x(oam, oam2, i);
      for (int j = oam_x; j < oam_x + 8; ++j) {
        buf[4 * (oam->y * SCREEN_WIDTH + j) + 2] = 255;
        buf[4 * ((oam->y + 7) * SCREEN_WIDTH + j) + 2] = 255;
      }
      for (int k = oam->y; k < oam->y + 8; ++k) {
        buf[4 * (k * SCREEN_WIDTH + oam_x) + 2] = 255;
        buf[4 * (k * SCREEN_WIDTH + oam_x + 7) + 2] = 255;
      }
    }
  }

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
//          dump_bg(*ppu0);
//          std::printf("\n");
          dump_bg(*ppu, 1);
          std::printf("\n");
//          dump_bg(*ppu, 2);
//          std::printf("\n");
//          std::exit(0);
          break;
        case SDLK_c:
          dump_colour_math(*ppu);
          break;
        case SDLK_p:
          dump_pal(*ppu);
          break;
        case SDLK_s:
          dump_sprite(*ppu);
          break;
        case SDLK_o: {
          bool dump_bytes = event.key.keysym.mod & KMOD_LCTRL;
          dump_oam(*ppu, dump_bytes);
          break;
        }

        case SDLK_BACKQUOTE:
          FLAGS_dis = !FLAGS_dis;
          break;

        case SDLK_SLASH:
          FLAGS_show_main = !FLAGS_show_main;
          break;

        case SDLK_PERIOD:
          FLAGS_show_sub = !FLAGS_show_sub;
          break;

        case SDLK_v:
          ppu->bus->pickle("save.state");
          break;
        case SDLK_e:
          ppu->bus->unpickle("save.state");
          break;

        case SDLK_r:
          ppu->bus->reset();
          break;
      }
    }
  }
}

void Screen::connect(std::shared_ptr<SPPU> p) {
  ppu = p;
}
