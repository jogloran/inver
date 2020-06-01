#include "ppu.hpp"

void
PPU::calculate_sprites() {
  if (!ppumask.show_sprites) return;

  auto height = ppuctrl.sprite_size ? 16 : 8;

  std::fill(shadow_oam.begin(), shadow_oam.end(), Sprite {{0xff, 0xff, 0xff, 0xff}, 0});

  // Determine sprite visibility for the next scanline by filling the shadow_oam
  byte next_scanline = scanline == -1 ? 0 : scanline + 1;

  candidate_sprites.clear();

  auto cur {oam.begin()};
  for (byte sprite_index = 0; cur != oam.end(); ++cur, ++sprite_index) {
    const auto& sprite = oam[sprite_index];
    if ((sprite.y >= 8 && next_scanline >= sprite.y &&
         next_scanline < sprite.y + height)) {
      candidate_sprites.push_back({sprite, sprite_index});
    }
  }

  std::copy_n(candidate_sprites.begin(), std::min<size_t>(candidate_sprites.size(), 8), shadow_oam.begin());
  if (candidate_sprites.size() > 8) {
    ppustatus.sprite_overflow = 1;
  }
}

inline void
PPU::cycle_start() {
  if (odd_frame) {
    ++ncycles;
  } else {
    odd_frame = !odd_frame;
  }
  frame_start = std::chrono::high_resolution_clock::now();
}

inline void
PPU::shift() {
  if (ppumask.show_background) {
    pt_shifter[0] <<= 1;
    pt_shifter[1] <<= 1;

    at_shifter[0] <<= 1;
    at_shifter[1] <<= 1;
  }
}

inline void
PPU::load_shift_reg() {
//  log("load_shift_reg: %02x %02x\n", bg_tile_lsb, bg_tile_msb);
  pt_shifter[0] = (pt_shifter[0] & 0xff00) | bg_tile_lsb;
  pt_shifter[1] = (pt_shifter[1] & 0xff00) | bg_tile_msb;

  at_shifter[0] = (at_shifter[0] & 0xff00) | at_byte_lsb;
  at_shifter[1] = (at_shifter[1] & 0xff00) | at_byte_msb;
}

inline void
PPU::nt_read() {
  load_shift_reg();
  nt_byte = ppu_read(0x2000 + (loopy_v.reg & 0xfff));
//  log("Read nt byte %02x from %04x\n", nt_byte, 0x2000 + (loopy_v.reg & 0xfff));
}

inline void
PPU::extra_nt_read() {
  nt_byte = ppu_read(0x2000 + (loopy_v.reg & 0xfff));
}

inline void
PPU::at_read() {
  auto at_byte = ppu_read(0x23c0 +
                          (loopy_v.nt_y << 11) +
                          (loopy_v.nt_x << 10) +
                          ((loopy_v.coarse_y >> 2) << 3) +
                          ((loopy_v.coarse_x >> 2)));
  // at_byte contains attribute information for a 16x16 region (2x2 tiles)

  // coarse_x and coarse_y are the same within an 8x8 pixel region, but we need to know
  // which quadrant of the 16x16 pixel region our coarse_x, coarse_y correspond to
  // TL: cx & 2 == 0, cy & 2 == 0 (fetch bits 1,0)
  // TR: cx & 2 == 1, cy & 2 == 0 (fetch bits 3,2)
  // BL: cx & 2 == 0, cy & 2 == 1 (fetch bits 5,4)
  // BR: cx & 2 == 1, cy & 2 == 1 (fetch bits 7,6)
  // We need to use the same at_byte for two tiles (i.e. every two values of cx, cy,
  // we will select one of four quadrants)
  if (loopy_v.coarse_y & 2) {
    at_byte >>= 4;
  }
  if (loopy_v.coarse_x & 2) {
    at_byte >>= 2;
  }
  at_byte &= 0x3;
  at_byte_msb = (at_byte & 2) ? 0xff : 0;
  at_byte_lsb = (at_byte & 1) ? 0xff : 0;
}

inline void
PPU::pt_read_lsb() {
  bg_tile_lsb = ppu_read(
      (ppuctrl.background_pattern_address << 12) + (nt_byte << 4) + loopy_v.fine_y);
}

inline void
PPU::pt_read_msb() {
  bg_tile_msb = ppu_read(
      (ppuctrl.background_pattern_address << 12) + (nt_byte << 4) + loopy_v.fine_y + 8);
}

inline void
PPU::scx() {
  if (ppumask.show_background) {
    if (loopy_v.coarse_x == 31) {
      loopy_v.coarse_x = 0;
      loopy_v.nt_x = ~loopy_v.nt_x;
    } else {
      ++loopy_v.coarse_x;
    }
  }
}

inline void
PPU::scy() {
  if (ppumask.show_background) {
    if (loopy_v.fine_y == 7) {
      loopy_v.fine_y = 0;
      if (loopy_v.coarse_y == 29) {
        loopy_v.coarse_y = 0;
        loopy_v.nt_y = ~loopy_v.nt_y;
      } else {
        ++loopy_v.coarse_y;
      }
    } else {
      ++loopy_v.fine_y;
    }
  }
}

inline void
PPU::cpx() {
  load_shift_reg();
  if (ppumask.show_background) {
    loopy_v.coarse_x = loopy_t.coarse_x;
    loopy_v.nt_x = loopy_t.nt_x;
  }
}

inline void
PPU::cpy() {
  if (ppumask.show_background) {
    loopy_v.fine_y = loopy_t.fine_y;
    loopy_v.coarse_y = loopy_t.coarse_y;
    loopy_v.nt_y = loopy_t.nt_y;
  }
}

inline void
PPU::set_vblank() {
  ppustatus.sprite0_hit = 0;
  ppustatus.sprite_overflow = 0;
  ppustatus.vblank_started = 1;
  if (ppuctrl.vblank_nmi) {
    nmi_req = true;
  }

//  auto now = std::chrono::high_resolution_clock::now();
//  std::chrono::duration<double> diff = std::chrono::duration_cast<std::chrono::duration<double>>(
//      now - last_frame);
//  log("frame: %fms\n", diff.count() * 1000);
//  last_frame = std::chrono::high_resolution_clock::now();
}

inline void
PPU::clr_vblank() {
  ppustatus.vblank_started = 0;
}

void
PPU::events_for(int s, int c) {
  if ((s == 241 || s == -1) && c == 1) {
    if (s == -1) clr_vblank(); else set_vblank();
  } else if (s == 0 && c == 0) {
    cycle_start();
  } else if (s <= 239 || s == -1) {
    if ((c >= 2 && c <= 257) || (c >= 321 && c <= 337)) {
      shift();
      switch ((c - 1) % 8) {
        case 0:
          nt_read();
          break;
        case 2:
          at_read();
          break;
        case 4:
          pt_read_lsb();
          break;
        case 6:
          pt_read_msb();
          break;
        case 7:
          scx();
          break;
      }
      if (c == 256) scy();
      else if (c == 257) cpx();

      if (s == 239 && c == 257) {
        frame_done();
      }
      if (c == 338 || c == 340) extra_nt_read();
    } else if (s == -1 && c >= 280 && c <= 304) {
      cpy();
    }
    if (s >= 0 && c == 304) {
      calculate_sprites();
    }
    if (s >= 0 && s <= 239 && c == 260) {
      if (ppumask.show_background || ppumask.show_sprites) {
        cart->signal_scanline();
      }
    }
  }
}

void
PPU::next_cycle() {
  ++ncycles;

  if (ncycles == 341) {
    ncycles = 0;
    ++scanline;
    if (scanline == 261) {
      scanline = -1;
    }
  }
}

void PPU::dump_oam() {
  int i = 0;
  for (OAM& sprite : oam) {
    if (sprite.y >= 0xef) {
      continue;
    }
    std::printf("%02d: (%02x) x=%02x y=%02x a=%02x %c\n", i, sprite.tile_no, sprite.x, sprite.y, sprite.attr,
                (sprite.attr & (1 << 5)) ? '*' : ' ');
    ++i;
  }
  std::printf("\n");
}

void PPU::dump_at() {
  std::printf("Attribute table bytes:\n");
  for (int row = 0; row < 8; ++row) {
    for (int col = 0; col < 16; ++col) {
      std::printf("%02x ", ppu_read((col < 8 ? 0x23c0 : 0x27c0) + row * 8 + (col % 8)));
    }
    std::printf("\n");
  }
  std::printf("\n");
  for (int row = 0; row < 16; ++row) {
    for (int col = 0; col < 16; ++col) {
      // TODO: this only works for vertical mirroring
      word base = col < 8 ? 0x23c0 : 0x27c0;
      byte pal_byte = ppu_read(base + (row / 2) * 8 + (col % 8));
      if (row % 2 == 0)
        std::printf("%01x %01x ", (pal_byte >> 0) & 0x3, (pal_byte >> 2) & 0x3);
      else
        std::printf("%01x %01x ", (pal_byte >> 4) & 0x3, (pal_byte >> 6) & 0x3);
    }
    std::printf("\n");
  }
  std::printf("\n");
}

void
PPU::tick() {
  events_for(scanline, ncycles);

  byte output = 0;
  byte output_palette = 0;
  if (ppumask.show_background) {
    // apply fine_x:
    // fine_x = 0 => take bit 7 of shifters
    // fine_x = 1 => take bit 6 ...
    //   ...  = 7 => take bit 0 ...
    // select one of the top 8 bits
    word mask = 0x8000 >> fine_x;
    output = !!(pt_shifter[0] & mask) | (!!(pt_shifter[1] & mask) << 1);
    // apply palette:
    // these select a set of colour indices:
    // bg palette 0: 0x3f01-0x3f03
    // bg palette 1: 0x3f05-0x3f07
    // bg palette 2: 0x3f09-0x3f0b
    // bg palette 3: 0x3f0d-0x3f0f
    output_palette = !!(at_shifter[0] & mask) | (!!(at_shifter[1] & mask) << 1);
  }

  byte bg = pal[0];
  if (scanline >= 0 && scanline <= 239 && ncycles >= 1 && ncycles <= 256) {
    if (ppumask.show_left_background || ncycles > 8) {
      screen.at(scanline * 256 + (ncycles - 1)) =
          output == 0 ? 0 : pal[4 * output_palette + output];
    }

    auto height = ppuctrl.sprite_size ? 16 : 8;

    if (ppumask.show_sprites && ncycles == 256) {
      std::sort(shadow_oam.begin(), shadow_oam.end(), [](const Sprite& s1, const Sprite& s2) {
        return s1.sprite_index < s2.sprite_index;
      });
      std::array<byte, 256> sprite_row;
      std::fill(sprite_row.begin(), sprite_row.end(), 0x0);
      for (const Sprite& sprite : shadow_oam) {
        auto visible = sprite.oam;
        if (visible.y >= 0xef) continue;

        for (int i = visible.x; i < visible.x + 8; ++i) {
          auto tile_no = visible.tile_no;
          bool y_mirrored = visible.attr & 0x80;

          byte y_selector =
              y_mirrored ? (7 - (scanline - visible.y) % 8) : (scanline - visible.y) % 8;

          bool select_top_half;
          word base_address;
          if (height == 16) {
            bool rendering_top_half = scanline - visible.y <= 7;
            // y-mirrored: true  rendering top: true  -> select_top_half: false
            //             false                true  -> select_top_half: true
            //             true                 false -> select_top_half: true
            //             false                false -> select_top_half: false
            select_top_half = !(y_mirrored ^ rendering_top_half);
            tile_no &= ~1;
            base_address = (visible.tile_no & 1) << 12;
          } else {
            select_top_half = false;
            base_address = ppuctrl.sprite_pattern_address << 12;
          }

          auto base_at_address {
              base_address + ((tile_no + select_top_half) << 4) +
              y_selector};
          byte lsb = ppu_read(base_at_address);
          byte msb = ppu_read(base_at_address + 8);

          auto decoded = unpack_bits(lsb, msb);
          auto sprite_palette = 4 + (visible.attr & 3);

          // apply horizontal flip if necessary
          byte selector = visible.attr & 0x40 ? (7 - (i - visible.x)) : i - visible.x;
          auto sprite_byte = decoded[selector];
          // TODO: the below causes problems
//          if (output != 0 && sprite_byte != 0) {
          if (ppumask.show_background && sprite_byte != 0 &&
              (ppumask.show_left_sprites || i >= 8) && i < 256) {
//            screen.fb[scanline * 256 + i] = pal[4 * sprite_palette + sprite_byte];
            if (sprite_row[i] == 0) {
              sprite_row[i] = pal[4 * sprite_palette + sprite_byte];
              if ((visible.attr & 0x20) && sprite_row[i] != 0) {
                sprite_row[i] += 64;
              }
            }
            LOG("drawing (x=% 2d, y=% 2d) %02x\n", i, scanline, sprite_byte);

            if (sprite.sprite_index == 0) {
              ppustatus.sprite0_hit = 1;
            }
          }
        }
      }

//      log("sprite row\n");
//      for (int i = 0; i < 256; ++i) {
//        std::printf("%02x", sprite_row[i]);
//      }
//      std::printf("\n");
//      log("fb row\n");
//      for (int i = 0; i < 256; ++i) {
//        std::printf("%02x", screen.fb[scanline * 256 + i]);
//      }
//      std::printf("\n");
//
//      for (int i = 0; i < 256; ++i) {
//        std::printf("%02x", sprite_row[i]);
//      }
//      std::printf("\n");

      // composite
      for (int i = 0; i < 256; ++i) {
        byte& b = screen.at(scanline * 256 + i);
        bool front_prio = sprite_row[i] < 64;

        if ((sprite_row[i] != 0 && front_prio) || b == 0) {
          b = sprite_row[i] % 64;
        }

        if (!b) {
          b = bg;
        }
      }
    }
  }

  next_cycle();
}

void PPU::frame_done() {
  auto now = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = std::chrono::duration_cast<std::chrono::duration<double>>(
      now - frame_start);
  screen.frame_rendered(diff.count() * 1000);
}
