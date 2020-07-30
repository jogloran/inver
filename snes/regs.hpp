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
};

union window_mask_op_t {
  enum class MaskOp : byte {
    Or = 0, And = 1, Xor = 2, Xnor = 3
  };
  struct {
    MaskOp bg1_op: 2;
    MaskOp bg2_op: 2;
    MaskOp bg3_op: 2;
    MaskOp bg4_op: 2;
  };
  byte reg;
};

struct window_t {
  enum class AreaSetting : byte {
    Disable, Inside, Outside
  };
  byte l;
  byte r; // inclusive
  AreaSetting mask_for_bg[4];
  AreaSetting mask_for_obj;
  AreaSetting mask_for_math;
  bool main_disable;
  bool sub_disable;
};

union vram_addr_t {
  struct {
    byte l;
    byte h;
  };
  word w;
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
};

union obsel_t {
  struct {
    byte obj_base_addr: 3;
    byte obj_gap_size: 2;
    byte obj_size: 3;
  };
  byte reg;
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
};

union vram_addr_incr_t {
  struct {
    byte step_mode: 2;
    byte addr_trans: 2; // TODO:
    byte unused: 3;
    byte after_accessing_high: 1; // Increment VRAM Address after accessing High/Low byte (0=Low, 1=High)
  };
  byte reg;
};

union bg_char_data_addr_t {
  struct {
    byte bg1_tile_base_addr: 4;
    byte bg2_tile_base_addr: 4;
  };
  byte reg;
};

union bg_base_size_t {
  struct {
    byte sc_size: 2;
    byte base_addr: 6;
  };
  byte reg;
};

union mosaic_t {
  struct {
    byte enable_for_bg: 4;
    byte size: 4;
  };
  byte reg;
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
};

union inidisp_t {
  struct {
    byte brightness: 4;
    byte unused: 3;
    byte force_blank: 1;
  };
  byte reg;
};

union layer_ctrl_t {
  struct {
    byte bg1: 1;
    byte bg2: 1;
    byte bg3: 1;
    byte bg4: 1;
    byte obj: 1;
    byte unused: 3;
  };
  byte reg;
};