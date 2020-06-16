#include "ppu.hpp"
#include "bus.hpp"

#include <chrono>

using namespace std::chrono_literals;

DECLARE_bool(tm);
DECLARE_bool(td);
DECLARE_int32(td_scanline);
DECLARE_int32(td_refresh_rate);

inline bool in(int start, int c, int end) {
  return c >= start && c <= end;
}

void
PPU::select(word ppu_cmd, byte value) {
  {
    switch (ppu_cmd) {
      case 0x0: { // ppuctrl
        auto old_nametable_base {ppuctrl.nt_base};
        ppuctrl.reg = value;
        loopy_t.nt = ppuctrl.nt_base;
        if (ppuctrl.nt_base != old_nametable_base) {
          bus->dump();
          log("Set nametable base %d -> %d\n", old_nametable_base, ppuctrl.nt_base);
        }
        break;
      }
      case 0x1: // ppumask
        ppumask.reg = value;
        break;
      case 0x2: // ppustatus
        break;
      case 0x3: // oamaddr
        break;
      case 0x4: // oamdata
        break;
      case 0x5: // ppuscroll
        if (!w) {
          loopy_t.coarse_x = value >> 3;
          fine_x = value & 7;
          LOG("scroll cx %d fx %d cy %d fy %d\n", loopy_t.coarse_x, fine_x, loopy_t.coarse_y,
              loopy_t.fine_y);
          w = 1;
        } else {
          loopy_t.coarse_y = value >> 3;
          loopy_t.fine_y = value & 7;
          LOG("scroll2 cy %d fy %d\n", loopy_t.coarse_y, loopy_t.fine_y);
          w = 0;
        }
        break;
      case 0x6: // ppuaddr
        log_select(ppu_cmd, "ppu addr write %02x\n", value);
        if (!w) {
          loopy_t.reg &= 0x3ff;
          loopy_t.reg = (loopy_t.reg & ~0x3f00) | ((value & 0x3f) << 8);
          w = 1;
        } else {
          loopy_t.reg = (loopy_t.reg & ~0xff) | value;
          loopy_v = loopy_t;
          w = 0;
        }

        break;
      case 0x7: // ppudata
        log_select(ppu_cmd, "ppu data write %02x -> %04x\n", value, loopy_v.reg);
        ppu_write(loopy_v.reg, value);
        loopy_v.reg += (ppuctrl.vram_increment ? 32 : 1);
        break;
      default:;
    }
  }
}

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
    if (sprite.y >= 0 && in(sprite.y, next_scanline, sprite.y + height - 1)) {
      candidate_sprites.push_back({sprite, sprite_index});
    }
  }

  std::copy_n(candidate_sprites.begin(), std::min<size_t>(candidate_sprites.size(), 8),
              shadow_oam.begin());
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
    pt.shift();
    at.shift();
  }
}

inline void
PPU::load_shift_reg() {
  pt.load(bg_tile_lsb, bg_tile_msb);
  at.load(at_byte_lsb, at_byte_msb);
}

inline void
PPU::nt_read() {
  load_shift_reg();
  nt_byte = ppu_read(0x2000 + (loopy_v.reg & 0xfff));
}

inline void
PPU::extra_nt_read() {
  nt_byte = ppu_read(0x2000 + (loopy_v.reg & 0xfff));
}

inline void
PPU::at_read() {
  auto at_byte = ppu_read(0x23c0 +
                          (loopy_v.nt << 10) +
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
  if (ppumask.show_background || ppumask.show_sprites) {
    if (loopy_v.coarse_x == 31) {
      loopy_v.reg ^= 0x41f;
    } else {
      ++loopy_v.coarse_x;
    }
  }
}

inline void
PPU::scy() {
  if (ppumask.show_background || ppumask.show_sprites) {
    if (loopy_v.fine_y == 7) {
      loopy_v.fine_y = 0;
      if (loopy_v.coarse_y == 31) {
        loopy_v.coarse_y = 0;
      } else if (loopy_v.coarse_y == 29) {
        loopy_v.coarse_y = 0;
        loopy_v.nt ^= 0b10;
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
  if (ppumask.show_background || ppumask.show_sprites) {
//    loopy_v.coarse_x = loopy_t.coarse_x;
//    loopy_v.nt_x = loopy_t.nt_x;
    loopy_v.reg = (loopy_v.reg & ~0x41f) | (loopy_t.reg & 0x41f);
  }
}

inline void
PPU::cpy() {
  if (ppumask.show_background || ppumask.show_sprites) {
//    loopy_v.fine_y = loopy_t.fine_y;
//    loopy_v.coarse_y = loopy_t.coarse_y;
//    loopy_v.nt_y = loopy_t.nt_y;
    loopy_v.reg = (loopy_v.reg & ~0x7be0) | (loopy_t.reg & 0x7be0);
  }
  std::fill(bg_is_transparent.begin(), bg_is_transparent.end(), 0x0);
}

inline void
PPU::set_vblank(bool b) {
  ppustatus.vblank_started = b;
  if (b) {
    ppustatus.sprite0_hit = 0;
    ppustatus.sprite_overflow = 0;
    if (ppuctrl.vblank_nmi) {
      nmi_req = true;
    }
  }
}

void
PPU::events_for(int s, int c) {
  if ((s == 241 || s == -1) && c == 1) set_vblank(s == 241);
  else if (s == 0 && c == 0) cycle_start();
  else if (s <= 239 || s == -1) {
    if (in(2, c, 257) || in(321, c, 337)) {
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

      if (s == 239 && c == 257) frame_done();
      if (c == 338 || c == 340) extra_nt_read();
    } else if (s == -1 && in(280, c, 304)) cpy();
    if (s >= 0 && c == 304) calculate_sprites();
    if (in(0, s, 239) && c == 260) {
      if (ppumask.show_background || ppumask.show_sprites) {
        cart->signal_scanline();
      }
    }
  }
  if (FLAGS_td && FLAGS_td_scanline == s && ncycles % FLAGS_td_refresh_rate == 0) {
    td.show();
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
    std::printf("%02d: (%02x) x=%02x y=%02x a=%02x %c\n", i, sprite.tile_no, sprite.x, sprite.y,
                sprite.attr,
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

#ifndef NDEBUG
  for (const auto& action : actions) {
    if (action.pred(*this)) {
      action.action(*this);
    }
  }
#endif

  byte output = 0;
  byte output_palette = 0;
  if (ppumask.show_background) {
    word mask = 0x8000 >> fine_x;
    output = pt(mask);
    output_palette = at(mask);
  }

  byte bg = pal[0];
  if (in(0, scanline, 239) && in(1, ncycles, 256)) {
    bool render_left_column = (ppumask.show_left_background || ncycles > 8);
    screen->at(scanline * 256 + (ncycles - 1)) =
        (output == 0 || !render_left_column) ? bg : pal[4 * output_palette + output];
    bg_is_transparent[ncycles - 1] = output == 0;

    auto height = ppuctrl.sprite_size ? 16 : 8;

    if (ppumask.show_sprites && ncycles == 256) {
      std::sort(shadow_oam.begin(), shadow_oam.end(), [](const Sprite& s1, const Sprite& s2) {
        return s1.sprite_index < s2.sprite_index;
      });
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
          if (ppumask.show_background && sprite_byte != 0 &&
              (ppumask.show_left_sprites || i >= 8) && i < 256) {
            if (sprite_row[i] == 0) {
              sprite_row[i] = pal[4 * sprite_palette + sprite_byte];
              if ((visible.attr & 0x20) && sprite_row[i] != 0) {
                // use bit 6 to encode that the sprite pixel has back priority
                sprite_row[i] |= 0x40;
              }
            }

            if (sprite.sprite_index == 0) {
              ppustatus.sprite0_hit = 1;
            }
          }
        }
      }

      for (int i = 0; i < 256; ++i) {
        byte& b = screen->at(scanline * 256 + i);
        bool front_prio = (sprite_row[i] & 0x40) == 0;

        // Allow the sprite pixel to be shown if:
        // - the sprite is opaque, and
        //     - the sprite has front priority over background tiles, or
        //     - the background pixel is transparent
        if (sprite_row[i] != 0 && (front_prio || bg_is_transparent[i])) {
          b = sprite_row[i] & 0x3f;
        }
      }
    }
  }

  next_cycle();
}

void PPU::frame_done() {
  auto now = std::chrono::high_resolution_clock::now();
  auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - frame_start);
  screen->frame_rendered(diff);
  if (FLAGS_tm) tm.show();
}
