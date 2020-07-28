#include <array>
#include <numeric>

#include "utils.hpp"
#include "sppu.hpp"
#include "bus_snes.hpp"
#include "ppu_utils.hpp"
#include "fort.hpp"

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

void SPPU::dump_bg(byte layer) {
  dword tilemap_base_addr = bg_base_size[layer].base_addr * 0x400;
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

std::array<byte, 256> composite(std::vector<std::array<byte, 256>> layers) {
  std::array<byte, 256> result = layers[0];
  for (auto it = layers.begin() + 1; it != layers.end(); ++it) {
    auto& layer = *it;
    for (int i = 0; i < 256; ++i) {
      auto& buf = result[i];
      auto& next = layer[i];

      if (buf % 8 == 0 && next % 8 != 0) {
        buf = next;
      }
    }
  }
  return result;
}

void SPPU::render_row() {
  auto bg1 = render_row(0);
  auto bg2 = render_row(1);
  auto bg3 = render_row(2);

  std::vector bgs {bg3, bg1, bg2};
  auto pals = composite(bgs);
  auto fb_ptr = screen->fb[0].data() + line * 256;

  for (byte pal_idx : pals) {
    Screen::colour_t rgb;
    if (pal_idx % 8 == 0) {
      rgb = backdrop_colour;
    } else {
      rgb = lookup(pal_idx);
    }

    fb_ptr->r = rgb.r;
    fb_ptr->g = rgb.g;
    fb_ptr->b = rgb.b;
    ++fb_ptr;
  }
}

std::array<byte, 256> SPPU::render_row(byte bg) {
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
  std::array<byte, 256 + 8> row;
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
                    *ptr++ = (1 << (2 * bpp)) * t->pal_no + pal_bytes[i];
                  }

                  ++col;
                });

  visible.clear();
  // Get OAM candidates based on current line
  // Render them into a structure RenderedSprite { oam, oam_index, pixels }
  // Sort to get priority
  // Draw onto row in that order
  OAM* oam_ptr = (OAM*) oam.data();
  for (byte i = 0; i < 128; ++i) {
    OAM* entry = &oam_ptr[i];
    OAM2* oam2 = (OAM2*) oam.data() + 512 + i / 4;
    auto oam_ = compute_oam_extras(entry, oam2, i);
    auto sprite_width = get_sprite_width(obsel.obj_size, oam_.is_large);
    auto sprite_height = get_sprite_height(obsel.obj_size, oam_.is_large);
    auto x_ = oam_.x_full;

    if (!(x_ >= 0 && x_ <= 255 && line >= entry->y && line < entry->y + sprite_height)) {
      continue;
    }

    std::vector<byte> pixels;
    auto tile_y = (line - entry->y) % 8; // the row (0..8) of the tile
    word tile_no = entry->tile_no + (entry->attr.tile_no_h << 8);

    word obj_char_data_base = obsel.obj_base_addr * 8192;
    for (int tile = 0; tile < sprite_width / 8; ++tile) {
      auto tile_no_x_offset = tile;
      if (entry->attr.flip_x) {
        tile_no_x_offset = sprite_width / 8 - tile - 1;
      }
      word obj_char_data_addr = obj_char_data_base
                              + (tile_no + tile_no_x_offset) * 0x10 // 8x8 tile row selector
          + tile_y // sub-tile row selector
          + 0x100 * ((line - entry->y) / 8); // 8x8 tile column selector
      auto pixel_array = decode_planar(&vram[obj_char_data_addr], 4, entry->attr.flip_x);

      // need to adjust pal_no for whichever tile it's in (horizontal and vertical)
      std::transform(pixel_array.begin(), pixel_array.end(), std::back_inserter(pixels),
                     [&entry](auto pal_index) {
                       return 128 + entry->attr.pal_no * 8 + pal_index;
                     });
    }
    visible.emplace_back(RenderedSprite {*entry, i, pixels});
  }

  // output row starting at scr[bg].x_reg % 8
  byte fine_offset = scr[bg].x_reg % 8;
//  auto* fb_ptr = screen->fb[bg].data() + line * 256;
  std::array<byte, 256> result;
  auto result_ptr = result.begin();
  for (int i = fine_offset; i < 256 + fine_offset; ++i) {
    *result_ptr++ = row[i];
  }

  for (auto& sprite : visible) {
    std::copy_n(sprite.pixels.begin(),
                std::min(sprite.pixels.size(), 256ul - sprite.oam.x),
                &result[sprite.oam.x]);
  }

  return result;

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

byte SPPU::get_sprite_width(byte obsel_size, byte is_large) {
  static std::array<byte, 16> table {
      8, 8, 8, 16, 16, 32, 16, 16,
      16, 32, 64, 32, 64, 64, 32, 32,
  };
  return table[(is_large ? 8 : 0) + obsel_size];
}

byte SPPU::get_sprite_height(byte obsel_size, byte is_large) {
  static std::array<byte, 16> table {
      8, 8, 8, 16, 16, 32, 32, 32,
      16, 32, 64, 32, 64, 64, 64, 32,
  };
  return table[(is_large ? 8 : 0) + obsel_size];
}

void SPPU::dump_oam(bool dump_bytes) {
  if (dump_bytes) {
    dump_oam_bytes();
  } else {
    dump_oam_table();
  }
}

void SPPU::dump_oam_bytes() {
  for (word addr = 0; addr < 0x220; ++addr) {
    if (addr % 0x20 == 0)
      std::printf("%03x | ", addr);

    auto byte = oam[addr];
//    if (byte == 0x0) {
//      std::printf("   ");
//    } else {
      std::printf("%02x ", byte);
//    }
    if (addr % 0x20 == 0x1f)
      std::printf("\n");
  }
  std::printf("\n");
}

void SPPU::dump_oam_table() {
  fort::char_table obsel_tb;
  obsel_tb << fort::header << "Attr" << "Value" << fort::endr;
  obsel_tb << "Size mode" << std::dec << int(obsel.obj_size) << fort::endr;
  obsel_tb << "Base addr" << std::hex << std::setw(4) << std::setfill('0') << obsel.obj_base_addr * 8192 << fort::endr;
  obsel_tb << "0xff-0x100 gap" << std::hex << std::setw(4) << std::setfill('0') << obsel.obj_gap_size * 4096 << fort::endr;

  std::cout << obsel_tb.to_string() << std::endl;

  fort::char_table tb;
  tb << fort::header << "#" << "X" << "Y" << "tile" << "prio" << "flipx" << "flipy" << "large" << "w" << "h" << "v" << fort::endr;

  OAM* oam_ptr = (OAM*) oam.data();
  for (byte i = 0; i < 128; ++i) {
    OAM* entry = &oam_ptr[i];
    OAM2* oam2 = (OAM2*) oam.data() + 512 + i / 4;

    auto oam_ = compute_oam_extras(entry, oam2, i);
    if (oam_.x_full == 0 && (entry->y == 0 || entry->y == 240)) continue;

    tb << std::dec << int(i) << int(oam_.x_full) << int(entry->y) << int(oam_.tile_no_full)
       << int(entry->attr.prio)
       << int(entry->attr.flip_x) << int(entry->attr.flip_y)
       << bool(oam_.is_large)
       << std::dec << int(get_sprite_width(obsel.obj_size, oam_.is_large))
       << std::dec << int(get_sprite_height(obsel.obj_size, oam_.is_large))
       << std::hex << std::setw(2) << std::setfill('0') << int(oam[512 + i / 4])
       << fort::endr;
  }

  std::cout << tb.to_string() << std::endl;
}
