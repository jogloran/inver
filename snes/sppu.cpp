#include <array>
#include <numeric>

#include "bus_snes.hpp"
#include "mode.hpp"
#include "ppu_utils.hpp"
#include "sppu.hpp"

DECLARE_bool(show_main);
DECLARE_bool(show_sub);

word SPPU::PAL_MASKED_IN_WINDOW = 257;
word SPPU::PAL_SUB_SCREEN_TRANSPARENT = 256;

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

using LayerPriorityTable = std::vector<LayerSpec>;
std::array<LayerPriorityTable, 8> prios_for_mode =
    {{// Mode 0
      {
          {3, 0},
          {2, 0},
          {Layers::OBJ, 0},
          {3, 1},
          {2, 1},
          {Layers::OBJ, 1},
          {1, 0},
          {0, 0},
          {Layers::OBJ, 2},
          {1, 1},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 1
      {
          {2, 0},
          {Layers::OBJ, 0},
          {Layers::OBJ, 1},
          {1, 0},
          {0, 0},
          {Layers::OBJ, 2},
          {1, 1},
          {0, 1},
          {Layers::OBJ, 3},
          {2, 1}},
      // Mode 2
      {
          {1, 0},
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {1, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 3
      {
          {1, 0},
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {1, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 4
      {
          {1, 0},
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {1, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 5
      {
          {1, 0},
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {1, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 6
      {
          {Layers::OBJ, 0},
          {0, 0},
          {Layers::OBJ, 1},
          {Layers::OBJ, 2},
          {0, 1},
          {Layers::OBJ, 3},
      },
      // Mode 7
      {}}};

const std::array<byte, 256>& SPPU::get_pal_row(const Layers& l, byte layer, byte prio) {
  if (layer == Layers::OBJ) {
    return l.obj.pal[prio];
  } else {
    return l.bg[layer].pal[prio];
  }
}

const Layers::Win& SPPU::get_mask_row(const Layers& l, byte layer) {
  if (layer == Layers::OBJ) {
    return l.obj.mask;
  } else {
    return l.bg[layer].mask;
  }
}

auto SPPU::prio_sort(const std::vector<LayerSpec>& prio, const Layers& l, int i) {
  return std::reduce(prio.rbegin(), prio.rend(),
                     std::make_tuple<byte, word, bool>(Layers::BACKDROP, 0, false),
                     [this, i, &l](auto acc,
                                   auto layer_spec) -> std::tuple<byte, word, bool> {
                       auto& [prio_layer, prio_pal, _] = acc;
                       auto& [layer, prio] = layer_spec;
                       auto& l2 = get_pal_row(l, layer, prio);
                       auto& mask = get_mask_row(l, layer);

                       if (!is_pal_clear(prio_pal)) {
                         return {prio_layer, prio_pal, mask[i]};
                       } else {
                         if (mask[i]) {
                           return {prio_layer, PAL_MASKED_IN_WINDOW, mask[i]};
                         } else {
                           if (!is_pal_clear(l2[i])) {
                             return {layer, l2[i], mask[i]};
                           } else {
                             return {prio_layer, prio_pal, mask[i]};
                           }
                         }
                       }
                     });
}

auto& SPPU::route_main_sub(std::vector<LayerSpec> prios) {
//  std::pair<std::vector<LayerSpec>, std::vector<LayerSpec>> result;
  main_sub.first.clear();
  main_sub.second.clear();
  for (LayerSpec l : prios) {
    switch (l.layer) {
      case 0:
      case 1:
      case 2:
      case 3:
        if (main_scr(l.layer)) main_sub.first.push_back(l);
        if (sub_scr(l.layer)) main_sub.second.push_back(l);
        break;

      case Layers::OBJ:
        if (main_scr.obj) main_sub.first.push_back(l);
        if (sub_scr.obj) main_sub.second.push_back(l);
        break;
    }
  }

  return main_sub;
}

void SPPU::render_row() {
  std::fill(main.begin(), main.end(), 0);
  std::fill(sub.begin(), sub.end(), PAL_SUB_SCREEN_TRANSPARENT);

  Layers l {mode_fns[bgmode.mode](*this)};
  auto prio = prios_for_mode[bgmode.mode];

  // Filter prio into MAIN and SUB
  auto [main_layers, sub_layers] = route_main_sub(prio);

  for (int i = 0; i < 256; ++i) {
    auto [main_layer, main_pal, main_masked] = prio_sort(main_layers, l, i);
    auto [sub_layer, sub_pal, sub_masked] = prio_sort(sub_layers, l, i);

    bool main_window_masked = main_layer == Layers::OBJ ? window_main_disable_mask.obj_disabled : (window_main_disable_mask.bg_disabled & (1 << main_layer));
    bool sub_window_masked = sub_layer == Layers::OBJ ? window_sub_disable_mask.obj_disabled : (window_sub_disable_mask.bg_disabled & (1 << sub_layer));

    main[i] = main_masked && main_window_masked ? 0 : main_pal;
    sub[i] = sub_masked && sub_window_masked ? PAL_SUB_SCREEN_TRANSPARENT : sub_pal;
    main_source_layer[i] = main_layer;
    sub_source_layer[i] = sub_layer;

    bool main_clear = is_pal_clear(main[i]);
    auto main_colour =
        main[i] == PAL_MASKED_IN_WINDOW ? Screen::colour_t {0, 0, 0} : lookup(main[i]);

    bool sub_clear = sub_source_layer[i] != Layers::BACKDROP && is_pal_clear(sub[i]);
    auto sub_colour =
        sub_source_layer[i] == Layers::BACKDROP ? backdrop_colour : lookup(sub[i]);
    // TODO: clearly not right, but the SMW dialog box needs a black bkgd
    if (main[i] == PAL_MASKED_IN_WINDOW) {
      pals[i] = lookup(0);
    } else if (sub_clear && main_clear) {
      pals[i] = lookup(0);
    } else if (sub_clear) {
      pals[i] = main_colour;
    } else if (main_clear) {
      pals[i] = sub_colour;
    } else {// !sub_clear && !main_clear
      if (colour_math_applies(i, l)) {
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
  auto fb_ptr = screen->fb[0].data() + line * 256;

  for (Screen::colour_t rgb : pals) {
    fb_ptr->r = rgb.r;
    fb_ptr->g = rgb.g;
    fb_ptr->b = rgb.b;
    ++fb_ptr;
  }
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
    pixels.clear();

    OAM* entry = &oam_ptr[i];
    if (entry->attr.prio != prio) continue;

    OAM2* oam2 = (OAM2*) oam.data() + 512 + i / 4;
    auto oam_ = compute_oam_extras(entry, oam2, i);
    auto [sprite_width, sprite_height] = get_sprite_dims(obsel.obj_size, oam_.is_large);
    auto x_ = oam_.x_full;

    if (!(x_ >= 0 && x_ <= 255 && line_ >= entry->y && line_ < entry->y + sprite_height)) {
      continue;
    }

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
      if (!is_pal_clear(sprite.pixels[i]))
        result[sprite.oam.x + i] = sprite.pixels[i];
    }
  }

  return result;
}

/**
 * Returns [cur_row, tile_row, line_] where:
 * - 0 <= cur_row < 64 is the vertical tile index of the current line, accounting for vertical scroll
 * - 0 <= tile_row < 7 is the y-offset into the 8 rows of the tile
 * - line_ is the vertical line number after adjusting for mosaicing
 * @param bg The background layer to evaluate
 */
auto SPPU::get_tile_pos(byte bg) {
  auto line_ = line;
  if (mosaic.enabled_for_bg(bg)) {
    line_ = (line / mosaic.size()) * (mosaic.size());
  }

  word y_offset = (line_ + scr[bg].y_reg) % 512;
  byte cur_row = y_offset / 8;
  byte tile_row = y_offset % 8;

  word x_offset = scr[bg].x_reg;
  byte cur_col = x_offset / 8;
  byte tile_col = x_offset % 8;

  return std::make_tuple(cur_row, tile_row, cur_col, tile_col, line_);
}

/**
 * prio can take values: 0,1 (bg or obj) 2,3 (obj)
 * in mode 1, BG3 can have priorities 0a, 0b as well as 1a, 1b.
 *   0, 1 refers to per-tile priority flag
 *   a, b refers to BG3 screen priority flag
 */
std::array<byte, 256> SPPU::render_row(byte bg, byte prio) {
  if (inidisp.force_blank) return {};

  // get bg mode
  byte mode = bgmode.mode;

  // 2bpp means one pixel is encoded in one word. wpp = words per pixel
  byte wpp = bpps_for_mode[mode][bg] / 2;

  dword tilemap_base_addr = bg_base_size[bg].base_addr * 0x400;
  dword chr_base_addr = bg_chr_base_addr_for_bg(bg);

  auto [cur_row, tile_row, cur_col, tile_col, line_] = get_tile_pos(bg);
  auto addrs = addrs_for_row(tilemap_base_addr, cur_col, cur_row, bg_base_size[bg].sc_size);

  // coming into this, we get 32 tile ids. for each tile id, we want to decode 8 palette values:
  int col = 0;
  auto ptr = row.begin();
  std::for_each(addrs.begin(), addrs.end(),
                [&, tile_row = tile_row](word addr) {
                  bg_map_tile_t* t = (bg_map_tile_t*) &vram[addr];
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
  for (int i = tile_col; i < 256 + tile_col; ++i) {
    *result_ptr++ = row[i];
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
            //            log("%-3ld x=%d line=%-3d vbl -> vis\n", x, ncycles, line);
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
      case 1:
      case 2:
      case 3:
        win[i] = ops[static_cast<byte>(bg_mask_op.bg_op(layer))](w1_in, w2_in);
        break;
      case Layers::OBJ:
        win[i] = ops[static_cast<byte>(obj_math_mask_op.bg_op(0))](w1_in, w2_in);
        break;
      case Layers::MATH:
        win[i] = ops[static_cast<byte>(obj_math_mask_op.bg_op(1))](w1_in, w2_in);
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

void SPPU::reset() {
  // TODO: not working yet
  main_scr.reg = 0;
  sub_scr.reg = 0;
  inidisp.reg = 0;
  bgmode.reg = 0;
  mosaic.reg = 0;
  bg_base_size[0].reg = 0;
  bg_base_size[1].reg = 0;
  bg_base_size[2].reg = 0;
  bg_base_size[3].reg = 0;
  bg_char_data_addr[0].reg = 0;
  bg_char_data_addr[1].reg = 0;
  vram_addr_incr.reg = 0;
  setini.reg = 0;
  obsel.reg = 0;
  oamadd.reg = 0;
  htime.reg = 0;
  vtime.reg = 0;
  vram_addr.w = 0;
  //  windows[0].reg = 0;
  //  windows[1].reg = 0;
  window_main_disable_mask.reg = 0;
  window_sub_disable_mask.reg = 0;
  bg_mask_op.reg = 0;
  obj_math_mask_op.reg = 0;
  cgwsel.reg = 0;
  colour_math.reg = 0;
  cgram_addr = 0;
  cgram_rw_upper = false;
  cgram_lsb = 0;
  std::fill(oam.begin(), oam.end(), 0);
  std::fill(vram.begin(), vram.end(), dual {});
  std::fill(pal.begin(), pal.end(), 0);
  std::fill(scr.begin(), scr.end(), BGScroll {});
  oam_lsb = 0;
  vram_prefetch.w = 0;
  sprite_range_overflow = false;
  hv_latched = false;
  hloc.w = 0;
  hloc_read_upper = false;
  vloc.w = 0;
  vloc_read_upper = false;
  backdrop_colour.reg = 0;
  last_mode = 0;
  ncycles = 0;
  line = 0;
  x = 0;
  visible.clear();
}
