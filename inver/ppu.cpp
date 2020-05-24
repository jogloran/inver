#include "ppu.hpp"

//std::array<Event, 14> PPU::procs {
//    &PPU::skip_cycle,
//    &PPU::nt_read,
//    &PPU::at_read,
//    &PPU::pt_read_lsb,
//    &PPU::pt_read_msb,
//    &PPU::scx,
//    &PPU::scy,
//    &PPU::cpx,
//    &PPU::cpy,
//    &PPU::set_vblank,
//    &PPU::clr_vblank,
//    &PPU::shift,
//    &PPU::extra_nt_read,
//    &PPU::calculate_sprites,
//};

void
PPU::calculate_sprites() {
  if (!ppumask.f.show_sprites) return;

  std::fill(shadow_oam.begin(), shadow_oam.end(), OAM {0xff, 0xff, 0xff, 0xff});

  // Determine sprite visibility for the next scanline by filling the shadow_oam
  byte next_scanline = scanline == -1 ? 0 : scanline + 1;
  std::vector<OAM> candidates;
  std::copy_if(oam.begin(), oam.end(), std::back_inserter(candidates),
               [this, next_scanline](auto sprite) {
                 return (sprite.y >= 8 && next_scanline >= sprite.y &&
                         next_scanline < sprite.y + 8);
               });
  std::copy_n(candidates.begin(), std::min<size_t>(candidates.size(), 8), shadow_oam.begin());
}

void
PPU::skip_cycle() {
  if (odd_frame) {
    ++ncycles;
  } else {
    odd_frame = !odd_frame;
  }
}

void
PPU::shift() {
  if (ppumask.f.show_background) {
    pt_shifter[0] <<= 1;
    pt_shifter[1] <<= 1;

    at_shifter[0] <<= 1;
    at_shifter[1] <<= 1;
  }
}

void
PPU::load_shift_reg() {
  pt_shifter[0] = (pt_shifter[0] & 0xff00) | bg_tile_lsb;
  pt_shifter[1] = (pt_shifter[1] & 0xff00) | bg_tile_msb;

  at_shifter[0] = (at_shifter[0] & 0xff00) | at_byte_lsb;
  at_shifter[1] = (at_shifter[1] & 0xff00) | at_byte_msb;
}

void
PPU::nt_read() {
  load_shift_reg();
  nt_byte = ppu_read(0x2000 + (loopy_v.reg & 0xfff));
//  log("Read nt byte %02x from %04x\n", nt_byte, 0x2000 + (loopy_v.reg & 0xfff));
}

void
PPU::extra_nt_read() {
  nt_byte = ppu_read(0x2000 + (loopy_v.reg & 0xfff));
}

void
PPU::at_read() {
  auto at_byte = ppu_read(0x23c0 + (ppuctrl.f.nametable_base << 8) +
                          ((loopy_v.f.coarse_y >> 2) << 3) +
                          ((loopy_v.f.coarse_x >> 2)));
  // at_byte contains attribute information for a 16x16 region (2x2 tiles)

  // coarse_x and coarse_y are the same within an 8x8 pixel region, but we need to know
  // which quadrant of the 16x16 pixel region our coarse_x, coarse_y correspond to
  // TL: cx & 2 == 0, cy & 2 == 0 (fetch bits 1,0)
  // TR: cx & 2 == 1, cy & 2 == 0 (fetch bits 3,2)
  // BL: cx & 2 == 0, cy & 2 == 1 (fetch bits 5,4)
  // BR: cx & 2 == 1, cy & 2 == 1 (fetch bits 7,6)
  // We need to use the same at_byte for two tiles (i.e. every two values of cx, cy,
  // we will select one of four quadrants)
  if (loopy_v.f.coarse_y & 2) {
    at_byte >>= 4;
  }
  if (loopy_v.f.coarse_x & 2) {
    at_byte >>= 2;
  }
  at_byte &= 0x3;
  at_byte_msb = (at_byte & 2) ? 0xff : 0;
  at_byte_lsb = (at_byte & 1) ? 0xff : 0;
}

void
PPU::pt_read_lsb() {
  bg_tile_lsb = ppu_read(
      (ppuctrl.f.background_pattern_address << 12) + (nt_byte << 4) + loopy_v.f.fine_y);
}

void
PPU::pt_read_msb() {
  bg_tile_msb = ppu_read(
      (ppuctrl.f.background_pattern_address << 12) + (nt_byte << 4) + loopy_v.f.fine_y + 8);
}

void
PPU::scx() {
  if (ppumask.f.show_background) {
    if (loopy_v.f.coarse_x == 31) {
      loopy_v.f.coarse_x = 0;
      loopy_v.f.nt_x = ~loopy_v.f.nt_x;
    } else {
      ++loopy_v.f.coarse_x;
    }
  }
}

void
PPU::scy() {
  if (ppumask.f.show_background) {
    if (loopy_v.f.fine_y == 7) {
      loopy_v.f.fine_y = 0;
      if (loopy_v.f.coarse_y == 29) {
        loopy_v.f.coarse_y = 0;
        loopy_v.f.nt_y = ~loopy_v.f.nt_y;
      } else {
        ++loopy_v.f.coarse_y;
      }
    } else {
      ++loopy_v.f.fine_y;
    }
  }
}

void
PPU::cpx() {
  load_shift_reg();
  if (ppumask.f.show_background) {
    loopy_v.f.coarse_x = loopy_t.f.coarse_x;
    loopy_v.f.nt_x = loopy_t.f.nt_x;
  }
}

void
PPU::cpy() {
  if (ppumask.f.show_background) {
    loopy_v.f.fine_y = loopy_t.f.fine_y;
    loopy_v.f.coarse_y = loopy_t.f.coarse_y;
    loopy_v.f.nt_y = loopy_t.f.nt_y;
  }
}

void
PPU::set_vblank() {
  ppustatus.f.vblank_started = 1;
  if (ppuctrl.f.vblank_nmi) {
    nmi_req = true;
  }

//  auto now = std::chrono::high_resolution_clock::now();
//  std::chrono::duration<double> diff = std::chrono::duration_cast<std::chrono::duration<double>>(
//      now - last_frame);
//  log("frame: %fms\n", diff.count() * 1000);
//  last_frame = std::chrono::high_resolution_clock::now();
}

void
PPU::clr_vblank() {
  ppustatus.f.vblank_started = 0;
}

void
PPU::push(Event event) {
  events.push_back(event);
}

void
PPU::events_for(int s, int c) {
  if ((s == 241 || s == -1) && c == 1) {
    if (s == -1) clr_vblank(); else set_vblank();
  } else if (s == 0 && c == 0) {
    skip_cycle();
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
      else if (c == 338 || c == 340) extra_nt_read();
    } else if (s == -1 && c >= 280 && c <= 304) {
      cpy();
    } else if (c == 304) {
      calculate_sprites();
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

void PPU::dump_at() {
  std::printf("Attribute table bytes:\n");
  for (word i = 0x23c0; i <= 0x23ff; ++i) {
    std::printf("%02x ", ppu_read(i));
    if (i % 8 == 7) {
      std::printf("\n");
    }
  }
  std::printf("\n\n");
  for (int row = 0; row < 16; ++row) {
    for (int col = 0; col < 8; ++col) {
      byte pal_byte = ppu_read(0x23c0 + (row / 2) * 8 + col);
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
  if (ppumask.f.show_background) {
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
    if (scanline == 8 && ncycles == 1 && output != 0) { ;
    }
    screen.fb[scanline * 256 + (ncycles - 1)] =
        output == 0 ? bg : pal[4 * output_palette + output];

    if (ppumask.f.show_sprites) {
      for (const OAM& visible : shadow_oam) {
        if (visible.y >= 0xef) continue;

        for (int i = visible.x; i < visible.x + 8; ++i) {
          byte y_selector =
              visible.attr & 0x80 ? (7 - (scanline - visible.y)) : scanline - visible.y;
          byte lsb = ppu_read(
              (ppuctrl.f.sprite_pattern_address << 12) + (visible.tile_no << 4) +
              y_selector);
          byte msb = ppu_read(
              (ppuctrl.f.sprite_pattern_address << 12) + (visible.tile_no << 4) +
              y_selector + 8);
//      log("visible.y %d scanline %d s-v.y-1 %d\n", visible.y, scanline, scanline - visible.y - 1);
          log("%02x sprite (x=% 2d, y=% 2d) %04x\n", visible.tile_no, visible.x, visible.y,
              (ppuctrl.f.sprite_pattern_address << 12) + (visible.tile_no << 4) +
              y_selector);

          // tile 0xa0 is at pattern table address 0x0a00 to 0xa0f
          auto decoded = unpack_bits(lsb, msb);
          auto sprite_palette = 4 + (visible.attr & 3);

          byte selector = visible.attr & 0x40 ? (7 - (i - visible.x)) : i - visible.x;
          auto sprite_byte = decoded[selector];
          if (sprite_byte != 0) {
            screen.fb[scanline * 256 + i] = pal[4 * sprite_palette + sprite_byte];
            log("drawing (x=% 2d, y=% 2d) %02x\n", i, scanline, sprite_byte);
          }
        }
      }
    }
  }

  next_cycle();
}
