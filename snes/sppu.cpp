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
    for (int cur_col = 0; cur_col < 64; ++cur_col) {
      word p = addr(tilemap_base_addr, cur_col, cur_row, true, true);
      bg_map_tile_t* t = (bg_map_tile_t*) &vram[p];

      if (t->char_no == 0xfc || t->char_no == 0xf8 || t->char_no == 0) {
        std::printf("        ");
      } else {
        std::printf("%3x%c(%1d) ", t->char_no, t->flip_x ? '*' : ' ', t->pal_no);
      }
    }
    std::printf("\n");
  }
}

std::array<byte, 256> composite(std::vector<std::array<byte, 256>> layers) {
  std::array<byte, 256> result = layers[0];
  int index = 0;
  for (auto it = layers.begin() + 1; it != layers.end(); ++it) {
    auto& layer = *it;
    for (int i = 0; i < 256; ++i) {
      auto& buf = result[i];
      auto& next = layer[i];

      if (buf == 0 && next != 0) {
        buf = next;
      }

      ++index;

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
    if (pal_idx == 0) {
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
  if (inidisp.force_blank) return {};

  if (bg == 0 && windows[0].l != 0 && windows[0].l < windows[0].r)
    std::printf("%3d wh0 %02x %02x\n", line, windows[0].l, windows[0].r);

  auto line_ = line;
  if (mosaic.enable_for_bg & (1 << bg)) {
    line_ = (line / (mosaic.size + 1)) * (mosaic.size + 1);
  }

  // get bg mode
  byte mode = bgmode.mode;

  // 2bpp means one pixel is encoded in one word. wpp = words per pixel
  byte wpp = bpps[bg] / 2;

  dword tilemap_base_addr = bg_base_size[bg].base_addr * 0x400;
  dword chr_base_addr = bg_chr_base_addr_for_bg(bg);

  byte cur_row = ((line_ + scr[bg].y_reg) % 512) / 8;
  byte tile_row = ((line_ + scr[bg].y_reg) % 512) % 8;

  word start_x_index = scr[bg].x_reg / 8;
  byte fine_x_offset = scr[bg].x_reg % 8;
  auto addrs = addrs_for_row(tilemap_base_addr, start_x_index, cur_row, bg_base_size[bg].sc_size);

  // no need to clear _tiles_ since we overwrite each one
  std::transform(addrs.begin(), addrs.end(), tiles.begin(), [this](auto addr) {
    return (bg_map_tile_t*) &vram[addr];
  });

  // coming into this, we get 32 tile ids. for each tile id, we want to decode 8 palette values:
  int col = 0;
  auto ptr = row.begin();
  std::for_each(tiles.begin(), tiles.end(),
                [&](bg_map_tile_t* t) {
                  auto tile_id = t->char_no;
                  auto row_to_access = tile_row;
                  if (t->flip_y)
                    row_to_access = 7 - row_to_access;

                  // get tile chr data
                  word tile_chr_base = tile_chr_addr(chr_base_addr, tile_id, row_to_access, wpp);

                  // decode planar data
                  // produce 8 byte values (palette indices)
                  std::array<byte, 8> pal_bytes = decode_planar(&vram[tile_chr_base], wpp,
                                                                t->flip_x);
                  for (int i = 0; i < 8; ++i) {
                    // Need to look up OAM to get the palette, then pal_bytes[i] gives an index
                    // into the palette

                    auto pal_index = (1 << (2 * wpp)) * t->pal_no + pal_bytes[i];
                    *ptr++ = (pal_index % (1 << (2 * wpp)) == 0) ? 0 : pal_index;
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
    auto[sprite_width, sprite_height] = get_sprite_dims(obsel.obj_size, oam_.is_large);
    auto x_ = oam_.x_full;

    if (!(x_ >= 0 && x_ <= 255 && line_ >= entry->y && line_ < entry->y + sprite_height)) {
      continue;
    }

    std::vector<byte> pixels;
    pixels.reserve(sprite_width);
    auto tile_y = (line_ - entry->y) % 8; // the row (0..8) of the tile
    auto tile_no_y_offset = (line_ - entry->y) / 8;
    if (entry->attr.flip_y) {
      tile_y = 7 - tile_y;
      tile_no_y_offset = sprite_height / 8 - tile_no_y_offset - 1;
    }
    word tile_no = entry->tile_no + (entry->attr.tile_no_h << 8);

    word obj_char_data_base = obsel.obj_base_addr * 8192;
    for (int tile = 0; tile < sprite_width / 8; ++tile) {
      auto tile_no_x_offset = tile;
      if (entry->attr.flip_x) {
        tile_no_x_offset = sprite_width / 8 - tile - 1;
      }
      word obj_char_data_addr = obj_addr(obj_char_data_base, tile_no, tile_no_x_offset,
                                         tile_no_y_offset, tile_y);
      auto pixel_array = decode_planar(&vram[obj_char_data_addr], 4, entry->attr.flip_x);

      std::transform(pixel_array.begin(), pixel_array.end(), std::back_inserter(pixels),
                     [&entry](auto pal_index) {
                       // All sprites are 16 colour (0 <= pal_index < 16) and all sub-tiles of a
                       // sprite share the same palette
                       return 128 + entry->attr.pal_no * 16 + pal_index;
                     });
    }
    visible.emplace_back(RenderedSprite {*entry, i, pixels});
  }

  std::array<byte, 256> result;
  auto result_ptr = result.begin();
  for (int i = fine_x_offset; i < 256 + fine_x_offset; ++i) {
    *result_ptr++ = row[i];
  }

  // TODO: need to look into priority for sprite pixels
  std::sort(visible.begin(), visible.end(), [](const RenderedSprite& t1, const RenderedSprite& t2) {
    return t1.oam_index == t2.oam_index
           ? (t1.oam.attr.prio > t2.oam.attr.prio)
           : t1.oam_index < t2.oam_index;
  });

  int nvisible = 0;
  for (auto& sprite : visible) {
    if (nvisible++ == 32) {
      sprite_range_overflow = true;
      break;
    }
    for (int i = 0; i < std::min(sprite.pixels.size(), 256ul - sprite.oam.x); ++i) {
      // Only replace non-transparent pixels. Since all sprite chr data is 4bpp, this means
      // a palette index of 0 within each palette.
      if (sprite.pixels[i] % 16 != 0)
        result[sprite.oam.x + i] = sprite.pixels[i];
    }
  }

  // Horizontal mosaic
  if (mosaic.enable_for_bg & (1 << bg)) {
    auto n = mosaic.size + 1;
    for (int i = 0; i < 256; ++i) {
      result[i] = result[(i / n) * n];
    }
  }

  return result;
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
        bus->hblank_start();
      }
      break;
    case State::HBLANK:
      if (ncycles >= 4 * 84) {
        ncycles -= 4 * 84;
        ++line;
        x = 0;

        if (line <= 0xe0) {
          state = State::VISIBLE;
        } else if (line == 0xe1) {
          state = State::VBLANK;
          bus->vblank_nmi();
        }
      }
      break;
    case State::VBLANK:
      if (ncycles >= 4 * 340) {
        ncycles -= 4 * 340;
        ++line;
        x = 0;
        // TODO: status flags need to distinguish hblank even during
        // vblank

        if (line >= 0x106) {
          vblank_end();
          screen->blit();
          state = State::VISIBLE;
          log("%-3ld x=%d line=%-3d vbl -> vis\n", x, ncycles, line);
          line = 0;

          bus->frame_start();
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
    std::printf("%02x ", byte);
    if (addr % 0x20 == 0x1f)
      std::printf("\n");
  }
  std::printf("\n");
}

void SPPU::dump_oam_table() {
  fort::char_table obsel_tb;
  obsel_tb << fort::header << "Attr" << "Value" << fort::endr;
  obsel_tb << "Size mode" << std::dec << int(obsel.obj_size) << fort::endr;
  obsel_tb << "Base addr" << std::hex << std::setw(4) << std::setfill('0')
           << obsel.obj_base_addr * 8192 << fort::endr;
  obsel_tb << "0xff-0x100 gap" << std::hex << std::setw(4) << std::setfill('0')
           << obsel.obj_gap_size * 4096 << fort::endr;

  std::cout << obsel_tb.to_string() << std::endl;

  fort::char_table tb;
  tb << fort::header << "#" << "X" << "Y" << "tile" << "pal" << "prio" << "flipx" << "flipy"
     << "large"
     << "w" << "h" << "v" << fort::endr;

  OAM* oam_ptr = (OAM*) oam.data();
  for (byte i = 0; i < 128; ++i) {
    OAM* entry = &oam_ptr[i];
    OAM2* oam2 = (OAM2*) oam.data() + 512 + i / 4;

    auto oam_ = compute_oam_extras(entry, oam2, i);
    if (oam_.x_full == 0 && (entry->y == 0 || entry->y == 240)) continue;

    auto[spr_width, spr_height] = get_sprite_dims(obsel.obj_size, oam_.is_large);

    tb << std::dec << int(i) << int(oam_.x_full) << int(entry->y)
       << int(oam_.tile_no_full) << int(entry->attr.pal_no)
       << int(entry->attr.prio)
       << int(entry->attr.flip_x) << int(entry->attr.flip_y)
       << bool(oam_.is_large)
       << std::dec << int(spr_width)
       << std::dec << int(spr_height)
       << std::hex << std::setw(2) << std::setfill('0') << int(oam[512 + i / 4])
       << fort::endr;
  }

  std::cout << tb.to_string() << std::endl;
}

void SPPU::vblank_end() {
  if (!inidisp.force_blank) sprite_range_overflow = false;
  bus->vblank_end();
}
