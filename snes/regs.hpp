#pragma once

#include "types.h"

struct BGScroll {
  void x(byte val) {
    if (bg_write_upper) {
      x_reg = (x_reg & ~0x300) | ((val & 3) << 8);
      bg_write_upper = false;
    } else {
      x_reg = (x_reg & ~0xff) | val;
      bg_write_upper = true;
    }
  }

  void y(byte val) {
    if (bg_write_upper) {
      y_reg = (y_reg & ~0x300) | ((val & 3) << 8);
      bg_write_upper = false;
    } else {
      y_reg = (y_reg & ~0xff) | val;
      bg_write_upper = true;
    }
  }

  word x_reg {};
  word y_reg {};
  bool bg_write_upper = false;

  template <typename Ar>
  void serialize(Ar& ar) { ar(x_reg, y_reg, bg_write_upper); }
};

union window_mask_op_t {
  enum class MaskOp : byte {
    Or = 0, And = 1, Xor = 2, Xnor = 3
  };
  byte reg;

  MaskOp bg_op(byte bg_layer) const {
    return static_cast<MaskOp>((reg >> (2 * bg_layer)) & 3);
  }

  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

struct window_t {
  enum class AreaSetting : byte {
    DisableInside, DisableOutside, Inside, Outside
  };
  byte l;
  byte r; // inclusive
  AreaSetting mask_for_bg[4];
  AreaSetting mask_for_obj;
  AreaSetting mask_for_math;
  template <typename Ar>
  void serialize(Ar& ar) { ar(l, r, mask_for_bg, mask_for_obj,
      mask_for_math); }
};

union window_disable_mask_t {
  struct {
    byte bg_disabled : 4;
    byte obj_disabled : 1;
    byte padding : 3;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union cgwsel_t {
  struct {
    bool direct_colour_enabled : 1;
    bool subscreen_bg_obj_enabled : 1;
    byte unused : 2;
    byte colour_math_enabled : 2;
    byte force_main_screen_black_flags : 2;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union cgadsub_t {
  struct {
    byte on_main_screen : 4;
    byte on_obj_4_to_7 : 1;
    byte on_backdrop : 1;
    byte half_result : 1;
    byte subtract : 1;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union vram_addr_t {
  struct {
    byte l;
    byte h;
  };
  word w;
  template <typename Ar>
  void serialize(Ar& ar) { ar(w); }
};

union hvtime_t {
  struct {
    byte l;
    byte h;
  };
  struct {
    word v: 9;
    byte unused: 7;
  };
  word reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

// each entry is 4 bytes
struct OAM {
  byte x;
  byte y;
  byte tile_no;
  union attr_t {
    struct {
      byte tile_no_h: 1;
      byte pal_no: 3;
      byte prio: 2;
      byte flip_x: 1;
      byte flip_y: 1;
    };
    byte reg;
  } attr;
  template <typename Ar>
  void serialize(Ar& ar) { ar(x, y, tile_no, attr.reg); }
};

struct OAM2 {
  byte obj0_xh: 1;
  byte obj0_sz: 1;
  byte obj1_xh: 1;
  byte obj1_sz: 1;
  byte obj2_xh: 1;
  byte obj2_sz: 1;
  byte obj3_xh: 1;
  byte obj3_sz: 1;
};

union oamadd_t {
  struct {
    word addr: 10; // Values of 0x220-0x3ff are mirrors of 0x200-0x21f
    byte unused: 5;
    byte obj_prio_rotate: 1;
  };
  word reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union obsel_t {
  struct {
    byte obj_base_addr: 3;
    byte obj_gap_size: 2;
    byte obj_size: 3;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union bg_map_tile_t {
  struct {
    word char_no: 10;
    byte pal_no: 3;
    byte bg_prio: 1;
    byte flip_x: 1;
    byte flip_y: 1;
  };
  word reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union setini_t {
  struct {
    byte vscan: 1;
    byte obj_v_direction_disp: 1;
    byte bg_v_direction_disp: 1;
    byte horiz_pseudo_512: 1;
    byte unused: 2;
    byte ext_bg: 1;
    byte ext_sync: 1;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union vram_addr_incr_t {
  struct {
    byte step_mode: 2;
    byte addr_trans: 2; // TODO:
    byte unused: 3;
    byte after_accessing_high: 1; // Increment VRAM Address after accessing High/Low byte (0=Low, 1=High)
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union bg_char_data_addr_t {
  struct {
    byte bg1_tile_base_addr: 4;
    byte bg2_tile_base_addr: 4;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union bg_base_size_t {
  struct {
    byte sc_size: 2;
    byte base_addr: 6;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union mosaic_t {
  struct {
    byte enable_for_bg: 4;
    byte size_minus_one: 4;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }

  bool enabled_for_bg(byte bg) const {
    return enable_for_bg & (1 << bg);
  }

  byte size() const {
    return size_minus_one + 1;
  }
};

union bgmode_t {
  struct {
    byte mode: 3;
    byte bg3_mode1_prio: 1;
    byte bg1_tile_size: 1; // TODO:
    byte bg2_tile_size: 1;
    byte bg3_tile_size: 1;
    byte bg4_tile_size: 1;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union inidisp_t {
  struct {
    byte brightness: 4;
    byte unused: 3;
    byte force_blank: 1;
  };
  byte reg;
  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union layer_ctrl_t {
  struct {
    byte bg: 4;
    byte obj: 1;
    byte unused: 3;
  };
  byte reg;

  bool operator()(byte bg_layer) const {
    return bg & (1 << bg_layer);
  }

  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};

union m7sel_t {
  struct {
    /* Controls what happens to tiles exceeding the 128x128 tile boundary:
     *   0, 1: Wrap
     *   2:    Transparent
     *   3:    Filled by tile ID 0x0 */
    byte screen_over: 2;
    byte unused: 4;
    byte screen_vflip: 1;
    byte screen_hflip: 1;
  };
  byte reg;

  template <typename Ar>
  void serialize(Ar& ar) { ar(reg); }
};
