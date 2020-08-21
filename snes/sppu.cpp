#include <array>
#include <numeric>

#include "bus_snes.hpp"
#include "fort.hpp"
#include "mode.hpp"
#include "ppu_utils.hpp"
#include "sppu.hpp"
#include "utils.hpp"

DECLARE_bool(show_main);
DECLARE_bool(show_sub);

static std::array<std::function<Screen::colour_t(Screen::colour_t,
                                                 Screen::colour_t)>,
                  4>
    cm_op {
        [](auto c1, auto c2) { return c1.add(c2, 1); },
        [](auto c1, auto c2) { return c1.add(c2, 2); },
        [](auto c1, auto c2) { return c1.sub(c2, 1); },
        [](auto c1, auto c2) { return c1.sub(c2, 2); },
    };

static std::array<std::function<Layers(SPPU&)>, 8> mode_fns {
    mode<0>, mode<1>, mode<2>, mode<3>, mode<4>, mode<5>, mode<6>, mode<7>};

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

auto SPPU::get_pal_row(const Layers& l, byte layer, byte prio) {
  if (layer == Layers::OBJ) {
    return l.obj.pal[prio];
  } else {
    return l.bg[layer].pal[prio];
  }
}

auto SPPU::get_mask_row(const Layers& l, byte layer) {
  if (layer == Layers::OBJ) {
    return l.obj.mask;
  } else {
    return l.bg[layer].mask;
  }
}

auto SPPU::prio_sort(std::vector<LayerSpec> prio, const Layers& l, int i) {
  return std::reduce(prio.rbegin(), prio.rend(),
                     std::make_tuple<byte, word, bool>(Layers::BACKDROP, 0, false),
                     [this, i, &l](auto acc,
                                   auto layer_spec) -> std::tuple<byte, word, bool> {
                       auto& [prio_layer, prio_pal, _] = acc;
                       auto& [layer, prio] = layer_spec;
                       auto l2 = get_pal_row(l, layer, prio);
                       auto mask = get_mask_row(l, layer);

                       if (prio_pal % 16 != 0) {
                         return {prio_layer, prio_pal, mask[i]};
                       } else {
                         if (mask[i]) {
                           return {prio_layer, 257, mask[i]};
                         } else {
                           if (l2[i] % 16 != 0) {
                             return {layer, l2[i], mask[i]};
                           } else {
                             return {prio_layer, prio_pal, mask[i]};
                           }
                         }
                       }
                     });
}

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

void SPPU::route_main_sub(std::tuple<byte, word, bool> prio_result, int i) {
  // applies TM, TS switch
  // need to apply TMW, TSW:
  // if TMW is set for BGx and _i_ is inside the window,
  // if main is on, set main to 0
  // if sub is on,  set sub to fixed colour

  auto [layer, pal, masked] = prio_result;
  bool main_window_masked = layer == Layers::OBJ ? window_main_disable_mask.obj_disabled : (window_main_disable_mask.bg_disabled & (1 << layer));
  bool sub_window_masked = layer == Layers::OBJ ? window_sub_disable_mask.obj_disabled : (window_sub_disable_mask.bg_disabled & (1 << layer));

  main_source_layer[i] = layer;// This is the culprit

  // TM, TS:
  // "If the layer data is prevented from propagating forward, the layer data is effectively
  //  disabled, and the layer is treated as transparent."

  // TMW: TSW:
  // Controls whether the window applies (after W1 and W2 are merged)

  // CGWSEL.1 (subscreen_bg_obj_enabled)
  // If false, SUB is treated as a solid background with the value of backdrop_colour

  // we need to treat differently these cases:
  // - backdrop pixel within window
  // - backdrop pixel outside window

  switch (layer) {
    case 0:
      if (main_scr.bg1) main[i] = (main_window_masked && masked) ? 0 : pal;
      if (sub_scr.bg1) sub[i] = (sub_window_masked && masked) ? 256 : pal;
      break;
    case 1:
      if (main_scr.bg2) main[i] = (main_window_masked && masked) ? 0 : pal;
      if (sub_scr.bg2) sub[i] = (sub_window_masked && masked) ? 256 : pal;
      break;
    case 2:
      if (main_scr.bg3) main[i] = (main_window_masked && masked) ? 0 : pal;
      if (sub_scr.bg3) sub[i] = (sub_window_masked && masked) ? 256 : pal;
      break;
    case 3:
      if (main_scr.bg4) main[i] = (main_window_masked && masked) ? 0 : pal;
      if (sub_scr.bg4) sub[i] = (sub_window_masked && masked) ? 256 : pal;
      break;
    case Layers::OBJ:
      if (main_scr.obj) main[i] = (main_window_masked && masked) ? 0 : pal;
      if (sub_scr.obj) sub[i] = (sub_window_masked && masked) ? 256 : pal;
      break;
    case Layers::BACKDROP:
      main[i] = 0;
      sub[i] = 256;
      break;
  }
}

auto SPPU::route_main_sub(std::vector<LayerSpec> prios) {
  std::pair<std::vector<LayerSpec>, std::vector<LayerSpec>> result;

  for (LayerSpec l : prios) {
    switch (l.layer) {
      case 0:
        if (main_scr.bg1) result.first.push_back(l);
        if (sub_scr.bg1) result.second.push_back(l);
        break;
      case 1:
        if (main_scr.bg2) result.first.push_back(l);
        if (sub_scr.bg2) result.second.push_back(l);
        break;
      case 2:
        if (main_scr.bg3) result.first.push_back(l);
        if (sub_scr.bg3) result.second.push_back(l);
        break;
      case 3:
        if (main_scr.bg4) result.first.push_back(l);
        if (sub_scr.bg4) result.second.push_back(l);
        break;
      case Layers::OBJ:
        if (main_scr.obj) result.first.push_back(l);
        if (sub_scr.obj) result.second.push_back(l);
        break;
    }
  }

  return result;
}

void SPPU::render_row() {
  std::array<Screen::colour_t, 256> pals;

  std::fill(main.begin(), main.end(), 0);
  std::fill(sub.begin(), sub.end(), 256);

  Layers l {mode_fns[bgmode.mode](*this)};
  switch (bgmode.mode) {
    case 0: {
      //      auto bg1 = render_row(0, 0);
      //      auto bg2 = render_row(1, 0);
      //      auto bg3 = render_row(2, 0);
      //      auto bg4 = render_row(3, 0);
      //
      //      std::vector bgs {bg3, bg1, bg2, bg4};
      //      pals = composite(bgs);
      //      break;
    }
    case 1: {
      // priority sorting (take topmost non-transparent pixel)
      //      std::vector prio {bg3, obj0, obj1, bg2, bg1, obj2, bg21, bg11, obj3, bg3b};
      std::vector<LayerSpec> prio {
          {2, 0},
          {Layers::OBJ, 0},
          {Layers::OBJ, 1},
          {1, 0},
          {0, 0},
          {Layers::OBJ, 2},
          {1, 1},
          {0, 1},
          {Layers::OBJ, 3},
          {2, 1}};

      //      for (int i = 0; i < 256; ++i) {
      //        pals[i] = lookup(l.bg[1].pal[0][i]);
      //      }
      //      break;

      // Filter prio into MAIN and SUB
      auto [main_layers, sub_layers] = route_main_sub(prio);

      // need to use colour math Layers::MATH (how?)
      // Colour math is enabled on backdrop (2131). What does this mean?
      // pixels inside the window which are on the backdrop layer should be set to black
      for (int i = 0; i < 256; ++i) {
        // Each pixel goes into one and only one layer here (is this right?) No.
        // Each layer is turned on or off for MAIN or SUB by TM/TS.
        // One MAIN topmost pixel will be computed
        // One SUB topmost pixel will be computed
        // If a layer is disabled for MAIN or SUB, it will not be considered in the prio list
        auto [main_layer, main_pal, main_masked] = prio_sort(main_layers, l, i);
        auto [sub_layer, sub_pal, sub_masked] = prio_sort(sub_layers, l, i);

        bool main_window_masked = main_layer == Layers::OBJ ? window_main_disable_mask.obj_disabled : (window_main_disable_mask.bg_disabled & (1 << main_layer));
        bool sub_window_masked = sub_layer == Layers::OBJ ? window_sub_disable_mask.obj_disabled : (window_sub_disable_mask.bg_disabled & (1 << sub_layer));

        main[i] = main_masked && main_window_masked ? 0 : main_pal;
        sub[i] = sub_masked && sub_window_masked ? 256 : sub_pal;
        main_source_layer[i] = main_layer;
        sub_source_layer[i] = sub_layer;
      }

      for (int i = 0; i < 256; ++i) {
        // if backdrop colour, then main[i] or sub[i] will == 256
        // in this case, if cgwsel.7 has 3, then set main[i] to black wherever it has 256
        bool main_clear = main[i] % 16 == 0;
        auto main_colour =
            main[i] == 257 ? Screen::colour_t {0, 0, 0} : lookup(main[i]);

        bool sub_clear = sub_source_layer[i] != Layers::BACKDROP && sub[i] % 16 == 0;
        auto sub_colour =
            sub_source_layer[i] == Layers::BACKDROP ? backdrop_colour : lookup(sub[i]);
        if (main[i] == 257) { // TODO: clearly not right, but the SMW dialog box needs a black bkgd
          pals[i] = lookup(0);
        } else if (sub_clear && main_clear) {
          pals[i] = lookup(0);
        } else if (sub_clear) {
          pals[i] = main_colour;
        } else if (main_clear) {
          pals[i] = sub_colour;
        } else {// !sub_clear && !main_clear

          // in lttp,
          // colour math is not applied when both layer bg1 (rain) and bg2 (world map) are
          // displayed, because main_clear is considered true.
          // what _should_ happen is that both bg1 and bg2 are considered non-clear
          // when bg2 alone is being drawn, main_clear and sub_clear are both false, so
          // colour math applies
          // when bg1 and bg2 are being drawn,
          // main[i] contains all 0s (WHY??)

          // Need to take into account:
          // 2130 (cgwsel)
          // are we in the colour math window? (2130.4,5 - CGWSEL)
          // 0: colour math always, 1: inside math window, 2: outside math window, 3: never
          if (colour_math_applies(i, l)) {
            // 2131 (colour_math)
            // does the pixel correspond to a layer that has colour math enabled? (2131.0-5 - CGADSUB)
            //   we have lost the layer information at this point
            // operation add/subtract? (2131.7)
            if (FLAGS_show_main) {
              pals[i] = main_colour;
            } else if (FLAGS_show_sub) {
              pals[i] = sub_colour;
            } else {
              pals[i] = cm_op[colour_math.reg >> 6](main_colour, sub_colour);
            }
          } else {
            pals[i] = main_colour;
          }
        }
      }

      // TM, TW
      //      pals = composite(bgs);
      // resolve to 15-bit colours
      // masking with window using TMW/TSW
      // main/sub routing
      // colour math to obtain final pixel
      break;
    }

    case 3: {
      //      auto bg1 = render_row(0, 0);
      //      auto bg2 = render_row(1, 0);
      //
      //      std::vector bgs {bg1, bg2};
      //      pals = composite(bgs);
      //      break;
    }
  }
  auto fb_ptr = screen->fb[0].data() + line * 256;

  // Need to compose the main and sub screens
  // before CM can be done

  for (Screen::colour_t rgb : pals) {
    //    Screen::colour_t rgb;
    //    if (pal_idx == 0) {
    //      rgb = backdrop_colour;
    //    } else {
    //      rgb = lookup(pal_idx);
    //    }

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
    static constexpr const char* ops[] = {"Main + Sub", "(Main + Sub) / 2", "Main - Sub",
                                          "(Main - Sub) / 2"};
    return ops[b >> 6];
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
  tb << header << ""
     << "BG1"
     << "BG2"
     << "BG3"
     << "BG4"
     << "OBJ"
     << "BD" << endr;
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
  auto main_screen_black =
      tb << "Main black (2130)" << fmt_cgwsel(cgwsel.force_main_screen_black_flags) << endr;
  auto cm_op = tb << "CM op (2131)" << fmt_cgadsub(colour_math.reg) << endr;
  auto backdrop = tb << separator << "Backdrop"
                     << std::hex << std::setfill('0') << std::setw(2) << int(backdrop_colour.r)
                     << std::hex << std::setfill('0') << std::setw(2) << int(backdrop_colour.g)
                     << std::hex << std::setfill('0') << std::setw(2) << int(backdrop_colour.b)
                     << endr;
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
    auto [sprite_width, sprite_height] = get_sprite_dims(obsel.obj_size, oam_.is_large);
    auto x_ = oam_.x_full;

    if (!(x_ >= 0 && x_ <= 255 && line_ >= entry->y && line_ < entry->y + sprite_height)) {
      continue;
    }

    std::vector<byte> pixels;
    pixels.reserve(sprite_width);
    auto tile_y = (line_ - entry->y) % 8;// the row (0..8) of the tile
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

  if (bg == 1) {
    ;
  }

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
    //    if (bg == 0 && (i > windows[0].l - fine_x_offset && i <= windows[0].r - fine_x_offset)) {
    //      *result_ptr++ = 3; // TODO: fake
    //    } else {
    *result_ptr++ = row[i];
    //    }
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
  for (int i = 0; i < master_cycles; ++i) {
    ++ncycles;

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
  obsel_tb << fort::header << "Attr"
           << "Value" << fort::endr;
  obsel_tb << "Size mode" << std::dec << int(obsel.obj_size) << fort::endr;
  obsel_tb << "Base addr" << std::hex << std::setw(4) << std::setfill('0')
           << obsel.obj_base_addr * 8192 << fort::endr;
  obsel_tb << "0xff-0x100 gap" << std::hex << std::setw(4) << std::setfill('0')
           << obsel.obj_gap_size * 4096 << fort::endr;

  std::cout << obsel_tb.to_string() << std::endl;

  fort::char_table tb;
  tb << fort::header << "#"
     << "X"
     << "Y"
     << "tile"
     << "pal"
     << "prio"
     << "flipx"
     << "flipy"
     << "large"
     << "w"
     << "h"
     << "v" << fort::endr;

  OAM* oam_ptr = (OAM*) oam.data();
  for (byte i = 0; i < 128; ++i) {
    OAM* entry = &oam_ptr[i];
    OAM2* oam2 = (OAM2*) oam.data() + 512 + i / 4;

    auto oam_ = compute_oam_extras(entry, oam2, i);
    if (oam_.x_full == 0 && (entry->y == 0 || entry->y == 240)) continue;

    auto [spr_width, spr_height] = get_sprite_dims(obsel.obj_size, oam_.is_large);

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

Layers::Win SPPU::compute_mask(byte layer) {
  static std::array<std::function<bool(bool, bool)>, 4> ops {
      std::logical_or<bool>(),
      std::logical_and<bool>(),
      std::bit_xor<bool>(),
      std::not_fn(std::bit_xor<bool>()),
  };
  Layers::Win win {};

  for (int i = 0; i < 256; ++i) {
    bool w1_in;
    bool w2_in;
    switch (windows[0].mask_for_bg[layer]) {
      case window_t::AreaSetting::Inside:
        w1_in = i >= windows[0].l && i <= windows[0].r;
        break;
      case window_t::AreaSetting::Outside:
        w1_in = i < windows[0].l || i > windows[0].r;
        break;
      case window_t::AreaSetting::DisableInside:
      case window_t::AreaSetting::DisableOutside:
        w1_in = false;
        break;
    }
    switch (windows[1].mask_for_bg[layer]) {
      case window_t::AreaSetting::Inside:
        w2_in = i >= windows[1].l && i <= windows[1].r;
        break;
      case window_t::AreaSetting::Outside:
        w2_in = i < windows[1].l || i > windows[1].r;
        break;
      case window_t::AreaSetting::DisableInside:
      case window_t::AreaSetting::DisableOutside:
        w2_in = false;
        break;
    }

    // TODO: this seems incorrect
    if (windows[0].r <= windows[0].l) {
      w1_in = false;
    }
    if (windows[1].r <= windows[1].l) {
      w2_in = false;
    }
    // combine the two windows using the op
    switch (layer) {
      case 0:
        win[i] = ops[static_cast<byte>(bg_mask_op.bg1_op)](w1_in, w2_in);
        break;
      case 1:
        win[i] = ops[static_cast<byte>(bg_mask_op.bg2_op)](w1_in, w2_in);
        break;
      case 2:
        win[i] = ops[static_cast<byte>(bg_mask_op.bg3_op)](w1_in, w2_in);
        break;
      case 3:
        win[i] = ops[static_cast<byte>(bg_mask_op.bg4_op)](w1_in, w2_in);
        break;
      case Layers::OBJ:
        win[i] = ops[static_cast<byte>(obj_math_mask_op.bg1_op)](w1_in, w2_in);
        break;
      case Layers::MATH:
        win[i] = ops[static_cast<byte>(obj_math_mask_op.bg2_op)](w1_in, w2_in);
        break;
    }
  }

  return win;
}

bool SPPU::colour_math_applies(int i, const Layers& layers) {
  // does the colour math window apply?
  auto enabled_by_math_window = [&]() -> bool {
    switch (cgwsel.colour_math_enabled) {
      case 0:
        return true;
      case 1:
        return layers.math[i];
      case 2:
        return !layers.math[i];
      case 3:
        return false;
    }
    return false;
  };
  auto math_window_enabled_for_layer = [&]() -> bool {
    auto layer = main_source_layer[i];
    switch (layer) {
      case 0:
      case 1:
      case 2:
      case 3:
        // in lttp, bg2 doesn't display because colour math is enabled on bg2
        return colour_math.on_main_screen & (1 << layer);
      case Layers::OBJ:
        // TODO: this gives wrong results for palettes 0-3, so disabling until we can address
        return false;//colour_math.on_obj_4_to_7;
      case Layers::BACKDROP:
        return colour_math.on_backdrop;
    }
    return false;
  };
  return enabled_by_math_window() && math_window_enabled_for_layer();
}
