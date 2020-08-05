#include <array>
#include <numeric>

#include "utils.hpp"
#include "sppu.hpp"
#include "bus_snes.hpp"
#include "ppu_utils.hpp"
#include "fort.hpp"

// table of (mode, layer) -> bpp?

std::array<std::array<byte, 4>, 8> bpps_for_mode = {{
                                                        {2, 2, 2, 2},
                                                        {4, 4, 2, 0},
                                                        {4, 4, 0, 0},
                                                        {8, 4, 0, 0},
                                                        {8, 2, 0, 0},
                                                        {4, 2, 0, 0},
                                                        {4, 0, 0, 0},
                                                        {0, 0, 0, 0},
                                                    }};

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

      if (next != 0) {
        buf = next;
      }

      ++index;

    }
  }
  return result;
}

void SPPU::render_row() {
  std::array<byte, 256> pals;
  switch (bgmode.mode) {
    case 0: {
      auto bg1 = render_row(0, 0);
      auto bg2 = render_row(1, 0);
      auto bg3 = render_row(2, 0);
      auto bg4 = render_row(3, 0);

      std::vector bgs {bg3, bg1, bg2, bg4};
      pals = composite(bgs);
      break;
    }
    case 1: {
      auto bg1 = render_row(0, 0);
      auto bg2 = render_row(1, 0);
      auto bg3 = render_row(2, 0);
      auto bg3b = render_row(2, 1);

      auto obj0 = render_obj(0);
      auto obj1 = render_obj(1);
      auto obj2 = render_obj(2);
      auto obj3 = render_obj(3);

//      std::vector bgs { bg3b, obj0, bg3, obj1, obj2, bg1, bg2, obj3 };
      std::vector bgs {obj0, bg3, obj1, bg2, bg1, obj2, obj3, bg3b};
      pals = composite(bgs);
      break;
    }

    case 3: {
      auto bg1 = render_row(0, 0);
      auto bg2 = render_row(1, 0);

      std::vector bgs {bg1, bg2};
      pals = composite(bgs);
      break;
    }
  }
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

void SPPU::dump_colour_math() {
  static auto fmt_bool = [](byte b) { return b == 0 ? "" : "X"; };
  static auto fmt_cgwsel = [](byte b) {
    static constexpr const char* cgwsel[] = {"Always", "Math Window", "~Math Window", "Never"};
    return cgwsel[b];
  };
  static auto fmt_cgadsub = [](byte b) {
    static constexpr const char* cgwsel[] = {"Main + Sub", "Main - Sub", "(Main + Sub) / 2",
                                             "(Main - Sub) / 2"};
    return cgwsel[b >> 6];
  };
  static auto fmt_wxlog = [](window_mask_op_t::MaskOp b) {
    static constexpr const char* op[] = {"Or", "And", "Xor", "Xnor"};
    return op[static_cast<int>(b)];
  };
  static auto fmt_wxsel = [](window_t::AreaSetting b) {
    static constexpr const char* op[] = {"", "", "In", "Out"};
    return op[static_cast<int>(b)];
  };
  using namespace fort;
  char_table tb;
  tb.set_border_style(FT_SOLID_ROUND_STYLE);
  tb.column(0).set_cell_text_align(text_align::right);
  tb << header << "" << "BG1" << "BG2" << "BG3" << "BG4" << "OBJ" << "BD" << endr;
  tb << "Main (212c)" << fmt_bool(main_scr.bg1) << fmt_bool(main_scr.bg2) << fmt_bool(main_scr.bg3)
     << fmt_bool(main_scr.bg4) << fmt_bool(main_scr.obj) << endr;
  tb << "Sub (212d)" << fmt_bool(sub_scr.bg1) << fmt_bool(sub_scr.bg2) << fmt_bool(sub_scr.bg3)
     << fmt_bool(sub_scr.bg4) << fmt_bool(sub_scr.obj) << endr;
  tb << "Win bounds (2126-9)" << int(windows[0].l) << int(windows[0].r) << "" << int(windows[1].l)
     << int(windows[1].r) << "" << endr;
  tb << "Main Win off (212e)" << fmt_bool(window_main_disable_mask.bg_disabled & 1)
     << fmt_bool(window_main_disable_mask.bg_disabled & 2)
     << fmt_bool(window_main_disable_mask.bg_disabled & 4)
     << fmt_bool(window_main_disable_mask.bg_disabled & 8)
     << fmt_bool(window_main_disable_mask.obj_disabled) << "" << endr;
  tb << "Sub Win off (212f)" << fmt_bool(window_sub_disable_mask.bg_disabled & 1)
     << fmt_bool(window_sub_disable_mask.bg_disabled & 2)
     << fmt_bool(window_sub_disable_mask.bg_disabled & 4)
     << fmt_bool(window_sub_disable_mask.bg_disabled & 8)
     << fmt_bool(window_sub_disable_mask.obj_disabled) << "" << endr;
  tb << "Win1+2 merge (2121a-b)" << fmt_wxlog(bg_mask_op.bg1_op)
     << fmt_wxlog(bg_mask_op.bg2_op)
     << fmt_wxlog(bg_mask_op.bg3_op)
     << fmt_wxlog(bg_mask_op.bg4_op)
     << fmt_wxlog(obj_math_mask_op.bg1_op)
     << fmt_wxlog(obj_math_mask_op.bg2_op)
     << endr;
  tb << "Win1 mask (2123-5)" << fmt_wxsel(windows[0].mask_for_bg[0])
     << fmt_wxsel(windows[0].mask_for_bg[1])
     << fmt_wxsel(windows[0].mask_for_bg[2])
     << fmt_wxsel(windows[0].mask_for_bg[3])
     << fmt_wxsel(windows[0].mask_for_obj)
     << fmt_wxsel(windows[0].mask_for_math)
     << endr;
  tb << "Win2 mask (2123-5)" << fmt_wxsel(windows[1].mask_for_bg[0])
     << fmt_wxsel(windows[1].mask_for_bg[1])
     << fmt_wxsel(windows[1].mask_for_bg[2])
     << fmt_wxsel(windows[1].mask_for_bg[3])
     << fmt_wxsel(windows[1].mask_for_obj)
     << fmt_wxsel(windows[1].mask_for_math)
     << endr;

  tb << "CM (2131)" << fmt_bool(colour_math.on_main_screen & 1)
     << fmt_bool(colour_math.on_main_screen & 2)
     << fmt_bool(colour_math.on_main_screen & 4)
     << fmt_bool(colour_math.on_main_screen & 8)
     << fmt_bool(colour_math.on_obj_4_to_7)
     << fmt_bool(colour_math.on_backdrop) << endr;
  auto cm_on = tb << separator << "CM on (2130)" << fmt_cgwsel(cgwsel.colour_math_enabled) << endr;
  auto main_screen_black = tb << "Main black (2130)" << fmt_cgwsel(cgwsel.force_main_screen_black_flags) << endr;
  auto cm_op = tb << "CM op (2131)" << fmt_cgadsub(colour_math.reg) << endr;
  auto backdrop = tb << separator << "Backdrop"
     << std::hex << std::setfill('0') << std::setw(2) << int(backdrop_colour.r)
     << std::hex << std::setfill('0') << std::setw(2) << int(backdrop_colour.g)
     << std::hex << std::setfill('0') << std::setw(2) << int(backdrop_colour.b) << endr;
  tb << "Subscreen BG/OBJ enable (2130)" << fmt_bool(cgwsel.subscreen_bg_obj_enabled) << endr;
  tb[10][1].set_cell_span(6);
  tb[11][1].set_cell_span(6);
  tb[12][1].set_cell_span(6);
  tb[13][3].set_cell_span(4);
  tb[14][1].set_cell_span(6);
  std::cout << tb.to_string() << std::endl;
}

std::array<byte, 256> SPPU::render_obj(byte prio) {
  if (inidisp.force_blank) return {};

  std::array<byte, 256> result {};
  auto line_ = line;

  visible.clear();
  // Get OAM candidates based on current line
  // Render them into a structure RenderedSprite { oam, oam_index, pixels }
  // Sort to get priority
  // Draw onto row in that order
  OAM* oam_ptr = (OAM*) oam.data();
  for (byte i = 0; i < 128; ++i) {
    OAM* entry = &oam_ptr[i];
    if (entry->attr.prio != prio) continue;

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
    word tile_no = oam_.tile_no_full;

    word obj_char_data_base = obsel.obj_base_addr << 13;
    for (int tile = 0; tile < sprite_width / 8; ++tile) {
      auto tile_no_x_offset = tile;
      if (entry->attr.flip_x) {
        tile_no_x_offset = sprite_width / 8 - tile - 1;
      }
      word obj_char_data_addr = obj_addr(obj_char_data_base, tile_no, tile_no_x_offset,
                                         tile_no_y_offset, tile_y);
      auto pixel_array = decode_planar(&vram[obj_char_data_addr], 2, entry->attr.flip_x);

      std::transform(pixel_array.begin(), pixel_array.end(), std::back_inserter(pixels),
                     [&entry](auto pal_index) {
                       // All sprites are 16 colour (0 <= pal_index < 16) and all sub-tiles of a
                       // sprite share the same palette
                       return 128 + entry->attr.pal_no * 16 + pal_index;
                     });
    }
    visible.emplace_back(RenderedSprite {*entry, i, pixels});
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

  return result;
}

/**
 * prio can take values: 0,1 (bg or obj) 2,3 (obj)
 * in mode 1, BG3 can have priorities 0a, 0b as well as 1a, 1b.
 *   0, 1 refers to per-tile priority flag
 *   a, b refers to BG3 screen priority flag
 */
std::array<byte, 256> SPPU::render_row(byte bg, byte prio) {
  if (inidisp.force_blank) return {};

  auto line_ = line;
  if (mosaic.enabled_for_bg(bg)) {
    line_ = (line / mosaic.size()) * (mosaic.size());
  }

  // get bg mode
  byte mode = bgmode.mode;

  // 2bpp means one pixel is encoded in one word. wpp = words per pixel
  byte wpp = bpps_for_mode[mode][bg] / 2;

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
                  if (t->bg_prio != prio) {
                    for (int i = 0; i < 8; ++i) {
                      *ptr++ = 0;
                    }
                    return;
                  }

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
                    // TODO: This seems to work in 8bpp, although I think that direct colour interferes
                    //  with t->pal_no
                    auto pal_index = (1 << (2 * wpp)) * t->pal_no + pal_bytes[i];
                    *ptr++ = (pal_index % (1 << (2 * wpp)) == 0) ? 0 : pal_index;
                  }

                  ++col;
                });

  std::array<byte, 256> result {};
  auto result_ptr = result.begin();
  for (int i = fine_x_offset; i < 256 + fine_x_offset; ++i) {
    if (bg == 0 && (i > windows[0].l - fine_x_offset && i <= windows[0].r - fine_x_offset)) {
      *result_ptr++ = 3; // TODO: fake
    } else {
      *result_ptr++ = row[i];
    }
//    *result_ptr++ = row[i];
  }

  // Horizontal mosaic
  if (mosaic.enabled_for_bg(bg)) {
    auto n = mosaic.size();
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
        } else if (line >= 0xe1) {
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
