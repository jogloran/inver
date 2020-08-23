#include "ppu_debug.hpp"

#include "bus_snes.hpp"
#include "fort.hpp"
#include "mode.hpp"
#include "ppu_utils.hpp"
#include "types.h"

void dump_colour_math(SPPU& sppu) {
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
  tb << "Main (212c)" << fmt_bool(sppu.main_scr(0)) << fmt_bool(sppu.main_scr(1)) << fmt_bool(sppu.main_scr(2))
     << fmt_bool(sppu.main_scr(3)) << fmt_bool(sppu.main_scr.obj) << endr;
  tb << "Sub (212d)" << fmt_bool(sppu.sub_scr(0)) << fmt_bool(sppu.sub_scr(1)) << fmt_bool(sppu.sub_scr(2))
     << fmt_bool(sppu.sub_scr(3)) << fmt_bool(sppu.sub_scr.obj) << endr;
  tb << "Win bounds (2126-9)" << int(sppu.windows[0].l) << int(sppu.windows[0].r) << "" << int(sppu.windows[1].l)
     << int(sppu.windows[1].r) << "" << endr;
  tb << "Main Win off (212e)" << fmt_bool(sppu.window_main_disable_mask.bg_disabled & 1)
     << fmt_bool(sppu.window_main_disable_mask.bg_disabled & 2)
     << fmt_bool(sppu.window_main_disable_mask.bg_disabled & 4)
     << fmt_bool(sppu.window_main_disable_mask.bg_disabled & 8)
     << fmt_bool(sppu.window_main_disable_mask.obj_disabled) << "" << endr;
  tb << "Sub Win off (212f)" << fmt_bool(sppu.window_sub_disable_mask.bg_disabled & 1)
     << fmt_bool(sppu.window_sub_disable_mask.bg_disabled & 2)
     << fmt_bool(sppu.window_sub_disable_mask.bg_disabled & 4)
     << fmt_bool(sppu.window_sub_disable_mask.bg_disabled & 8)
     << fmt_bool(sppu.window_sub_disable_mask.obj_disabled) << "" << endr;
  tb << "Win1+2 merge (2121a-b)" << fmt_wxlog(sppu.bg_mask_op.bg_op(0))
     << fmt_wxlog(sppu.bg_mask_op.bg_op(1))
     << fmt_wxlog(sppu.bg_mask_op.bg_op(2))
     << fmt_wxlog(sppu.bg_mask_op.bg_op(3))
     << fmt_wxlog(sppu.obj_math_mask_op.bg_op(0))
     << fmt_wxlog(sppu.obj_math_mask_op.bg_op(1))
     << endr;
  tb << "Win1 mask (2123-5)" << fmt_wxsel(sppu.windows[0].mask_for_bg[0])
     << fmt_wxsel(sppu.windows[0].mask_for_bg[1])
     << fmt_wxsel(sppu.windows[0].mask_for_bg[2])
     << fmt_wxsel(sppu.windows[0].mask_for_bg[3])
     << fmt_wxsel(sppu.windows[0].mask_for_obj)
     << fmt_wxsel(sppu.windows[0].mask_for_math)
     << endr;
  tb << "Win2 mask (2123-5)" << fmt_wxsel(sppu.windows[1].mask_for_bg[0])
     << fmt_wxsel(sppu.windows[1].mask_for_bg[1])
     << fmt_wxsel(sppu.windows[1].mask_for_bg[2])
     << fmt_wxsel(sppu.windows[1].mask_for_bg[3])
     << fmt_wxsel(sppu.windows[1].mask_for_obj)
     << fmt_wxsel(sppu.windows[1].mask_for_math)
     << endr;

  tb << "CM (2131)" << fmt_bool(sppu.colour_math.on_main_screen & 1)
     << fmt_bool(sppu.colour_math.on_main_screen & 2)
     << fmt_bool(sppu.colour_math.on_main_screen & 4)
     << fmt_bool(sppu.colour_math.on_main_screen & 8)
     << fmt_bool(sppu.colour_math.on_obj_4_to_7)
     << fmt_bool(sppu.colour_math.on_backdrop) << endr;
  auto cm_on = tb << separator << "CM on (2130)" << fmt_cgwsel(sppu.cgwsel.colour_math_enabled) << endr;
  auto main_screen_black =
      tb << "Main black (2130)" << fmt_cgwsel(sppu.cgwsel.force_main_screen_black_flags) << endr;
  auto cm_op = tb << "CM op (2131)" << fmt_cgadsub(sppu.colour_math.reg) << endr;
  auto backdrop = tb << separator << "Backdrop"
                     << std::hex << std::setfill('0') << std::setw(2) << int(sppu.backdrop_colour.r)
                     << std::hex << std::setfill('0') << std::setw(2) << int(sppu.backdrop_colour.g)
                     << std::hex << std::setfill('0') << std::setw(2) << int(sppu.backdrop_colour.b)
                     << endr;
  tb << "Subscreen BG/OBJ enable (2130)" << fmt_bool(sppu.cgwsel.subscreen_bg_obj_enabled) << endr;
  tb[10][1].set_cell_span(6);
  tb[11][1].set_cell_span(6);
  tb[12][1].set_cell_span(6);
  tb[13][3].set_cell_span(4);
  tb[14][1].set_cell_span(6);
  std::cout << tb.to_string() << std::endl;
}

void dump_sprite(SPPU& sppu) {
  word addr = 0x27c0 / 2;
  for (int i = 0; i < 32; ++i) {
    std::printf("%04x ", sppu.vram[addr + i].w);
  }
  std::printf("\n");
  for (int i = 0; i < 8; ++i) {
    auto row = decode_planar(&sppu.vram[addr + i], 2, false);
    for (byte b : row) {
      std::printf("%-2x ", static_cast<int>(b));
    }
    std::printf("\n");
  }
}

void dump_pal(SPPU& sppu) {
  for (auto it = sppu.pal.begin(); it != sppu.pal.end(); it += 2) {
    if (std::distance(sppu.pal.begin(), it) % 32 == 0) {
      std::printf("\n%04x | ", std::distance(sppu.pal.begin(), it));
    }

    std::printf("%02x %02x ", *it, *(it + 1));
  }
  std::printf("\n");
}

void dump_bg(SPPU& sppu, byte layer) {
  dword tilemap_base_addr = sppu.bg_base_size[layer].base_addr * 0x400;
  for (int cur_row = 0; cur_row < 64; ++cur_row) {
    for (int cur_col = 0; cur_col < 64; ++cur_col) {
      word p = addr(tilemap_base_addr, cur_col, cur_row, true, true);
      bg_map_tile_t* t = (bg_map_tile_t*) &sppu.vram[p];

      if (t->char_no == 0xfc || t->char_no == 0xf8 || t->char_no == 0) {
        std::printf("        ");
      } else {
        std::printf("%3x%c(%1d) ", t->char_no, t->flip_x ? '*' : ' ', t->pal_no);
      }
    }
    std::printf("\n");
  }
}

void dump_oam_bytes(SPPU& sppu) {
  for (word addr = 0; addr < 0x220; ++addr) {
    if (addr % 0x20 == 0)
      std::printf("%03x | ", addr);

    auto byte = sppu.oam[addr];
    std::printf("%02x ", byte);
    if (addr % 0x20 == 0x1f)
      std::printf("\n");
  }
  std::printf("\n");
}

void dump_oam_table(SPPU& sppu) {
  fort::char_table obsel_tb;
  obsel_tb << fort::header << "Attr"
           << "Value" << fort::endr;
  obsel_tb << "Size mode" << std::dec << int(sppu.obsel.obj_size) << fort::endr;
  obsel_tb << "Base addr" << std::hex << std::setw(4) << std::setfill('0')
           << sppu.obsel.obj_base_addr * 8192 << fort::endr;
  obsel_tb << "0xff-0x100 gap" << std::hex << std::setw(4) << std::setfill('0')
           << sppu.obsel.obj_gap_size * 4096 << fort::endr;

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

  OAM* oam_ptr = (OAM*) sppu.oam.data();
  for (byte i = 0; i < 128; ++i) {
    OAM* entry = &oam_ptr[i];
    OAM2* oam2 = (OAM2*) sppu.oam.data() + 512 + i / 4;

    auto oam_ = compute_oam_extras(entry, oam2, i);
    if (oam_.x_full == 0 && (entry->y == 0 || entry->y == 240)) continue;

    auto [spr_width, spr_height] = get_sprite_dims(sppu.obsel.obj_size, oam_.is_large);

    tb << std::dec << int(i) << int(oam_.x_full) << int(entry->y)
       << int(oam_.tile_no_full) << int(entry->attr.pal_no)
       << int(entry->attr.prio)
       << int(entry->attr.flip_x) << int(entry->attr.flip_y)
       << bool(oam_.is_large)
       << std::dec << int(spr_width)
       << std::dec << int(spr_height)
       << std::hex << std::setw(2) << std::setfill('0') << int(sppu.oam[512 + i / 4])
       << fort::endr;
  }

  std::cout << tb.to_string() << std::endl;
}

void dump_oam(SPPU& sppu, bool dump_bytes) {
  if (dump_bytes) {
    dump_oam_bytes(sppu);
  } else {
    dump_oam_table(sppu);
  }
}
