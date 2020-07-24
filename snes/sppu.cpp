#include <array>

#include "sppu.hpp"
#include "bus_snes.hpp"
#include "ppu_utils.hpp"

// table of (mode, layer) -> bpp?

std::array<byte, 3> bpps = {{4, 4, 2}};

void SPPU::dump_sprite() {
  word addr = 0x27c0 / 2;
  for (int i = 0; i < 32; ++i) {
    std::printf("%04x ", vram[addr + i].w);
  }
  std::printf("\n");
  for (int i = 0; i < 8; ++i) {
    auto row = decode_planar(&vram[addr + i], 2, false);
    for (byte b : row) {
      std::printf("%-2x ", static_cast<int>(b));
    }
    std::printf("\n");
  }
}

void SPPU::dump_pal() {
  for (auto it = pal.begin(); it != pal.end(); it += 2) {
    if (std::distance(pal.begin(), it) % 32 == 0) {
      std::printf("\n%04x | ", std::distance(pal.begin(), it));
    }

    std::printf("%02x %02x ", *it, *(it + 1));
  }
  std::printf("\n");
}

void SPPU::dump_bg() {
  dword tilemap_base_addr = bg_base_size[0].base_addr * 0x400;
  for (int cur_row = 0; cur_row < 64; ++cur_row) {
//    dword tilemap_offs_addr = tilemap_base_addr + cur_row * 32; // TODO: need to account for scroll
    std::printf("%04x ", addr(tilemap_base_addr, 0, cur_row, true, true));

    for (int cur_col = 0; cur_col < 64; ++cur_col) {
      word p = addr(tilemap_base_addr, cur_col, cur_row, true, true);
      SPPU::bg_map_tile_t* t = (SPPU::bg_map_tile_t*) &vram[p];

      if (t->char_no == 0xfc || t->char_no == 0xf8 || t->char_no == 0) {
        std::printf("        ");
      } else {
        std::printf("%3x%c(%1d) ", t->char_no, t->flip_x ? '*' : ' ', t->pal_no);
      }
    }
    std::printf("\n");

//    std::printf("     ");
//    for (int cur_col = 0; cur_col < 64; ++cur_col) {
//      word p = addr(tilemap_base_addr, cur_col, cur_row, true, true);
//      SPPU::bg_map_tile_t* t = (SPPU::bg_map_tile_t*) &vram[p];
//
//      if (t->reg != 0) {
//        std::printf("%-4x ", t->reg);
//      } else {
//        std::printf("     ");
//      }
//    }
//    std::printf("\n");
//
//    std::printf("     ");
//    for (int cur_col = 0; cur_col < 64; ++cur_col) {
//      word p = addr(tilemap_base_addr, cur_col, cur_row, true, true);
//      SPPU::bg_map_tile_t* t = (SPPU::bg_map_tile_t*) &vram[p];
//
//      if (t->pal_no != 0) {
//        std::printf("%-4x ", t->pal_no);
//      } else {
//        std::printf("     ");
//      }
//    }
//
//    std::printf("\n");
  }
}

word SPPU::addr(word base, word x, word y, bool sx, bool sy) {
  return (base
          + ((y & 0x1f) << 5)
          + (x & 0x1f)
          + (sy ? ((y & 0x20) << (sx ? 6 : 5)) : 0)
          + (sy ? ((x & 0x20) << 5) : 0));
}

std::array<word, 33> SPPU::addrs_for_row(word base, word start_x, word start_y) {
  std::array<word, 33> result;
  auto it = result.begin();
  for (int i = 0; i < 33; ++i) {
    *it++ = addr(base, (start_x + i) % 64, start_y, true, true);
  }
  return result;
}

void SPPU::render_row() {
  render_row(0);
  render_row(1);
  render_row(2);
}

void SPPU::render_row(byte bg) {
  // get bg mode
  byte mode = bgmode.mode;

  byte bpp = bpps[bg]; // 2bpp means one pixel is encoded in one word
  bpp /= 2;

  // assume we're looking at 4bpp layer 0
  // get base address for this layer
  dword tilemap_base_addr = bg_base_size[bg].base_addr * 0x400;

  // 1024 words after this addr correspond to the current tilemap
  byte cur_row = ((line + scr[bg].y_reg) % 512) / 8;
  byte tile_row = ((line + scr[bg].y_reg) % 512) % 8;

  dword chr_base_addr = bg_chr_base_addr_for_bg(bg);

  // starting x index = (scr[bg].x_reg / 8)
  word start_x_index = (scr[bg].x_reg / 8);
//  std::cout << start_x_index << ' ' << scr[bg].x_reg << '\n';
  auto addrs = addrs_for_row(tilemap_base_addr, start_x_index, cur_row);
  std::array<SPPU::bg_map_tile_t*, 33> tiles;
  std::transform(addrs.begin(), addrs.end(), tiles.begin(), [this](auto addr) {
    return (SPPU::bg_map_tile_t*) &vram[addr];
  });

  // coming into this, we get 32 tile ids. for each tile id, we want to decode 8 palette values:
  int col = 0;
  std::array<Screen::colour_t, 256 + 8> row;
  auto ptr = row.begin();
  std::for_each(tiles.begin(), tiles.end(),
                [&](SPPU::bg_map_tile_t* t) {
                  auto tile_id = t->char_no;

                  auto row_to_access = tile_row;
                  if (t->flip_y)
                    row_to_access = 7 - row_to_access;

                  // get tile chr data
                  word tile_chr_base = chr_base_addr + (8 * bpp) * tile_id + row_to_access;
//                  std::printf("> %06x (base %06x tile_id %04x tile_row %02x)\n", tile_chr_base,
//                      chr_base_addr, tile_id, tile_row);

                  // decode planar data
                  // produce 8 byte values (palette indices)
                  std::array<byte, 8> pal_bytes = decode_planar(&vram[tile_chr_base], bpp,
                                                                t->flip_x);
                  for (int i = 0; i < 8; ++i) {
                    // Need to look up OAM to get the palette, then pal_bytes[i] gives an index
                    // into the palette
                    Screen::colour_t rgb = lookup((1 << (2 * bpp)) * t->pal_no + pal_bytes[i]);
                    *ptr++ = rgb;
                  }

                  ++col;
                });

  // output row starting at scr[bg].x_reg % 8
  byte fine_offset = scr[bg].x_reg % 8;
  auto* fb_ptr = screen->fb[bg].data() + line * 256;// + col * 8;
  for (int i = fine_offset; i <= 256 + fine_offset; ++i) {
    fb_ptr->r = row[i].r;
    fb_ptr->g = row[i].g;
    fb_ptr->b = row[i].b;
    ++fb_ptr;
  }

  // determine which tiles are in viewport
  // 0...32

  // for each layer:
  // lookup bpp for mode and layer
  // write row of 256 to screen->fb[0]
}

Screen::colour_t SPPU::lookup(byte i) {
  word rgb = pal[2 * i] + (pal[2 * i + 1] << 8);
  return {
      .r = static_cast<byte>(rgb & 0x1f),
      .g = static_cast<byte>((rgb >> 5) & 0x1f),
      .b = static_cast<byte>((rgb >> 10) & 0x1f)};
}

void SPPU::tick(byte master_cycles) {
  ncycles += master_cycles;

  switch (state) {
    case State::VISIBLE:
      x = ncycles / 4;
      if (ncycles >= 4 * 256) {
        ncycles -= 4 * 256;
        state = State::HBLANK;

        render_row();
//        log("%-3ld x=%d line=%-3d vis -> hbl\n", x, ncycles, line);
      }
      break;
    case State::HBLANK:
      if (ncycles >= 84 * 4) {
        ncycles -= 84 * 4;
        ++line;
        x = 0;

        if (line <= 0xe0) {
          state = State::VISIBLE;
//          log("%-3ld x=%d line=%-3d hbl -> vis\n", x, ncycles, line);
        } else if (line == 0xe1) {
          state = State::VBLANK;
//          log("%-3ld x=%d line=%-3d hbl -> vbl\n", x, ncycles, line);
          bus->vblank_nmi();
        }
      }
      break;
    case State::VBLANK:
      if (ncycles >= 340 * 4) {
        ncycles -= 340 * 4;
        ++line;
        x = 0;
        // TODO: status flags need to distinguish hblank even during
        // vblank

        if (line >= 0x106) {
          bus->vblank_end();
          screen->blit();
          state = State::VISIBLE;
          log("%-3ld x=%d line=%-3d vbl -> vis\n", x, ncycles, line);
          line = 0;
        }
      }
      break;
  }

  auto hv_irq = bus->nmi.hv_irq;
  bool fire_irq = false;
  if (hv_irq == 1 && x == htime.v) {
    fire_irq = true;
  } else if (hv_irq == 2 && x == 0 && line == vtime.v) {
    fire_irq = true;
  } else if (hv_irq == 3 && x == htime.v && line == vtime.v) {
    fire_irq = true;
  }
  if (fire_irq) {
    bus->raise_timeup();
  }
}
