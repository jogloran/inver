#pragma once

#include <array>
#include "types.h"
#include "logger.hpp"
#include "screen.hpp"

#include <gflags/gflags.h>

DECLARE_bool(test_rom_output);

class BusSNES;

class Screen;

class SPPU : public Logger<SPPU> {
public:
  void connect(BusSNES* b) {
    bus = b;
  }

  void connect(std::shared_ptr<Screen> s) {
    screen = s;
  }

  void tick(byte master_cycles = 1);

  byte read(word addr) {
    switch (addr) {
      case 0x2134: // MPYL    - PPU1 Signed Multiply Result   (lower 8bit)
      case 0x2135: // MPYM    - PPU1 Signed Multiply Result   (middle 8bit)
      case 0x2136: // MPYH    - PPU1 Signed Multiply Result   (upper 8bit)
        break;

      case 0x2137: // SLHV    - PPU1 Latch H/V-Counter by Software (Read=Strobe)
        hv_latched = true;
        hloc = x;
        vloc = line;
        return 0x21;

      case 0x2138: // RDOAM   - PPU1 OAM Data Read            (read-twice)
        if (oamadd.addr < 0x200) {
          return oam[oamadd.addr++];
        } else {
          return oam[oamadd.addr++ % 0x21f];
        }

      case 0x2139: // RDVRAML - PPU1 VRAM Data Read           (lower 8bits)
        return vram_prefetch.l;

      case 0x213A: // RDVRAMH - PPU1 VRAM Data Read           (upper 8bits)
        return vram_prefetch.h;

      case 0x213B: // RDCGRAM - PPU2 CGRAM Data Read (Palette)(read-twice)
        byte value;
        if (cgram_rw_upper) {
          value = pal[2 * cgram_addr + 1];
        } else {
          value = pal[2 * cgram_addr];
        }
        cgram_rw_upper = !cgram_rw_upper;
        return value;

      case 0x213C: // OPHCT   - PPU2 Horizontal Counter Latch (read-twice)
        if (hloc_read_upper) {
          return hloc & 0xff;
        } else {
          return hloc >> 8;
        }
      case 0x213D: // OPVCT   - PPU2 Vertical Counter Latch   (read-twice)
        if (vloc_read_upper) {
          return vloc & 0xff;
        } else {
          return vloc >> 8;
        }
      case 0x213E: // STAT77  - PPU1 Status and PPU1 Version Number
        // TODO: set sprite overflow flags
        return 0x1 | (sprite_range_overflow << 6);
      case 0x213F: // STAT78  - PPU2 Status and PPU2 Version Number
      {
        // TODO: interlacing bit
        hloc_read_upper = vloc_read_upper = false;
        auto value = 0b0011 | (hv_latched << 6);
        hv_latched = false;
        return value;
      }
      default:;
    }

    return 0;
  }

  void write(word addr, byte value) {
    switch (addr) {
      case 0x2100: // INIDISP - Display Control 1
        inidisp.reg = value;
        screen->set_brightness(inidisp.brightness);
        break;

      case 0x2101: // OBSEL   - Object Size and Object Base
        obsel.reg = value;
        std::printf("sprite base address %04x\n", obsel.obj_base_addr * 8192);
        break;

      case 0x2102: // OAMADDL - OAM Address (lower 8bit)
        oamadd.reg = (oamadd.reg & 0xff00) | value;
        break;

      case 0x2103: // OAMADDH - OAM Address (upper 1bit) and Priority Rotation
        oamadd.reg = (oamadd.reg & 0xff) | (value << 8);
        break;

      case 0x2104: // OAMDATA - OAM Data Write (write-twice)
      {
        word oam_addr = oamadd.addr;

        if (oam_addr >= 0x200) {
          log("OAM[%04x] hi <- %02x\n", oam_addr, (value << 8));
          oam[oam_addr & 0x21f] = value;
        } else if ((oam_addr & 1) && oam_addr < 0x200) {
          log("OAM[%04x] <- %04x\n", oam_addr - 1, oam_lsb | (value << 8));
          oam[oam_addr] = value;
          oam[oam_addr - 1] = oam_lsb;
        } else {
          oam_lsb = value;
        }
        ++oamadd.addr;

        break;
      }

      case 0x2105: // BGMODE  - BG Mode and BG Character Size
        bgmode.reg = value;
        if (bgmode.mode != last_mode) {
          log_with_tag("bgmode", "bg mode: %d\n", bgmode.mode);
          last_mode = bgmode.mode;
        }
        break;

      case 0x2106: // MOSAIC  - Mosaic Size and Mosaic Enable
        mosaic.reg = value;
        break;

      case 0x2107: // BG1SC   - BG1 Screen Base and Screen Size
      case 0x2108: // BG2SC   - BG2 Screen Base and Screen Size
      case 0x2109: // BG3SC   - BG3 Screen Base and Screen Size
      case 0x210A: // BG4SC   - BG4 Screen Base and Screen Size
      {
        bg_base_size[addr - 0x2107].reg = value;
        // How many bytes/words is the tilemap? 32*32=0x400=1024 words=2048 bytes
        // VRAM has 0x8000=32768 words=65536 bytes
        // each word has structure bg_map_tile_t:
        //   Bit 0-9   - Character Number (000h-3FFh)
        //   Bit 10-12 - Palette Number   (0-7)
        //   Bit 13    - BG Priority      (0=Lower, 1=Higher)
        //   Bit 14    - X-Flip           (0=Normal, 1=Mirror horizontally)
        //   Bit 15    - Y-Flip           (0=Normal, 1=Mirror vertically)

        log("bg tilemap bases: 0: %06x 1: %06x 2: %06x 3: %06x\n",
            bg_base_size[0].base_addr * 0x400,
            bg_base_size[1].base_addr * 0x400,
            bg_base_size[2].base_addr * 0x400,
            bg_base_size[3].base_addr * 0x400);
        log("bg tilemap size: 0: %02x 1: %02x 2: %02x 3: %02x\n",
            bg_base_size[0].sc_size,
            bg_base_size[1].sc_size,
            bg_base_size[2].sc_size,
            bg_base_size[3].sc_size);
        break;
      }
      case 0x210B: // BG12NBA - BG Character Data Area Designation
      case 0x210C: // BG34NBA - BG Character Data Area Designation
        bg_char_data_addr[addr - 0x210b].reg = value;
        log("bg chr data bases: 0: %06x 1: %06x 2: %06x 3: %06x\n",
            bg_char_data_addr[0].bg1_tile_base_addr << 12,
            bg_char_data_addr[0].bg2_tile_base_addr << 12,
            bg_char_data_addr[1].bg1_tile_base_addr << 12,
            bg_char_data_addr[1].bg2_tile_base_addr << 12);
        break;

      case 0x210D: // BG1HOFS - BG1 Horizontal Scroll (X) (write-twice) / M7HOFS
      case 0x210E: // BG1VOFS - BG1 Vertical Scroll (Y)   (write-twice) / M7VOFS
      case 0x210F: // BG2HOFS - BG2 Horizontal Scroll (X) (write-twice)
      case 0x2110: // BG2VOFS - BG2 Vertical Scroll (Y)   (write-twice)
      case 0x2111: // BG3HOFS - BG3 Horizontal Scroll (X) (write-twice)
      case 0x2112: // BG3VOFS - BG3 Vertical Scroll (Y)   (write-twice)
      case 0x2113: // BG4HOFS - BG4 Horizontal Scroll (X) (write-twice)
      case 0x2114: // BG4VOFS - BG4 Vertical Scroll (Y)   (write-twice)
      {
        auto& reg = scr[(addr - 0x210d) / 2];
        if (addr & 1) {
          reg.x(value);
          if (reg.x_reg != 0)
            log_with_tag("scr", "%d %04x bg%d y = %04x\n", reg.bg_write_upper, addr,
                         (addr - 0x210d) / 2,
                         reg.x_reg);
        } else {
          reg.y(value);
          if (reg.y_reg != 0)
            log_with_tag("scr", "%d %04x bg%d x = %04x\n", reg.bg_write_upper, addr,
                         (addr - 0x210d) / 2,
                         reg.y_reg);
        }
        break;
      }

      case 0x2115: // VMAIN   - VRAM Address Increment Mode
        vram_addr_incr.reg = value;
        break;

      case 0x2116: // VMADDL  - VRAM Address (lower 8bit)
        vram_addr.l = value;
        vram_prefetch = vram[vram_addr.w];
        log("setting vram addr lo -> %04x\n", vram_addr);
        break;
      case 0x2117: // VMADDH  - VRAM Address (upper 8bit)
        vram_addr.h = value;
        vram_prefetch = vram[vram_addr.w];
        log("setting vram addr hi -> %04x\n", vram_addr);
        break;

      case 0x2118: // VMDATAL - VRAM Data Write (lower 8bit)
        vram[vram_addr.w & 0x7fff].l = value;
//        printf("2118 writing to %04x lo <- %02x\n", vram_addr.w & 0x7fff, value);
//        printf("2118 After writing: %04x\n", vram[vram_addr.w & 0x7fff].w);
        if (!vram_addr_incr.after_accessing_high) {
          vram_addr.w += vram_incr_step[vram_addr_incr.step_mode];
//          printf("2118 incr to %04x\n", vram_addr.w & 0x7fff);
        }
        break;
      case 0x2119: // VMDATAH - VRAM Data Write (upper 8bit)
        vram[vram_addr.w & 0x7fff].h = value;
//        printf("2119 writing to %04x hi <- %02x\n", vram_addr.w & 0x7fff, value);
//        printf("2119 After writing: %04x\n", vram[vram_addr.w & 0x7fff].w);
        if (vram_addr_incr.after_accessing_high) {
          vram_addr.w += vram_incr_step[vram_addr_incr.step_mode];
//          printf("2119 incr to %04x\n", vram_addr.w & 0x7fff);
        }
        break;

      case 0x211A: // M7SEL   - Rotation/Scaling Mode Settings
      case 0x211B: // M7A     - Rotation/Scaling Parameter A & Maths 16bit operand
      case 0x211C: // M7B     - Rotation/Scaling Parameter B & Maths 8bit operand
      case 0x211D: // M7C     - Rotation/Scaling Parameter C         (write-twice)
      case 0x211E: // M7D     - Rotation/Scaling Parameter D         (write-twice)
      case 0x211F: // M7X     - Rotation/Scaling Center Coordinate X (write-twice)
      case 0x2120: // M7Y     - Rotation/Scaling Center Coordinate Y (write-twice)
        break;

      case 0x2121: // CGADD   - Palette CGRAM Address
        log("2121 <- %02x\n", value);
        cgram_addr = value;
        cgram_rw_upper = false;
        break;
      case 0x2122: // CGDATA  - Palette CGRAM Data Write             (write-twice)
        log("2122 <- %02x\n", value);
        if (cgram_rw_upper) {
          log("pal write %02d <- %04x\n", cgram_addr, cgram_lsb + ((value & 0x7f) << 8));
          pal[2 * cgram_addr] = cgram_lsb;
          pal[2 * cgram_addr + 1] = value & 0x7f;
          ++cgram_addr;
        } else {
          cgram_lsb = value;
        }
        cgram_rw_upper = !cgram_rw_upper;
        break;

      case 0x2123: // W12SEL  - Window BG1/BG2 Mask Settings
        windows[0].mask_for_bg[0] = static_cast<window_t::AreaSetting>(value & 3);
        windows[1].mask_for_bg[0] = static_cast<window_t::AreaSetting>((value >> 2) & 3);
        windows[0].mask_for_bg[1] = static_cast<window_t::AreaSetting>((value >> 4) & 3);
        windows[1].mask_for_bg[1] = static_cast<window_t::AreaSetting>((value >> 6) & 3);
        break;
      case 0x2124: // W34SEL  - Window BG3/BG4 Mask Settings
        windows[0].mask_for_bg[2] = static_cast<window_t::AreaSetting>(value & 3);
        windows[1].mask_for_bg[2] = static_cast<window_t::AreaSetting>((value >> 2) & 3);
        windows[0].mask_for_bg[3] = static_cast<window_t::AreaSetting>((value >> 4) & 3);
        windows[1].mask_for_bg[3] = static_cast<window_t::AreaSetting>((value >> 6) & 3);
        break;
      case 0x2125: // WOBJSEL - Window OBJ/MATH Mask Settings
        windows[0].mask_for_obj = static_cast<window_t::AreaSetting>(value & 3);
        windows[1].mask_for_math = static_cast<window_t::AreaSetting>((value >> 2) & 3);
        break;
      case 0x2126: // WH0     - Window 1 Left Position (X1)
        log_with_tag("win", "win %d l pos %02x\n", 0, value);
        windows[0].l = value;
        break;
      case 0x2127: // WH1     - Window 1 Right Position (X2)
        log_with_tag("win", "win %d r pos %02x\n", 0, value);
        windows[0].r = value;
        break;

      case 0x2128: // WH2     - Window 2 Left Position (X1)
        log_with_tag("win", "win %d l pos %02x\n", 1, value);
        windows[1].l = value;
        break;
      case 0x2129: // WH3     - Window 2 Right Position (X2)
        log_with_tag("win", "win %d r pos %02x\n", 1, value);
        windows[1].r = value;
        break;

      case 0x212A: // WBGLOG  - Window 1/2 Mask Logic (BG1-BG4)
        bg_mask_op.reg = value;
        break;
      case 0x212B: // WOBJLOG - Window 1/2 Mask Logic (OBJ/MATH)
        obj_math_mask_op.reg = value;
        break;

      case 0x212C: // TM      - Main Screen Designation
        main_scr.reg = value;
        break;

      case 0x212D: // TS      - Sub Screen Designation
        sub_scr.reg = value;
        break;

      case 0x212E: // TMW     - Window Area Main Screen Disable
      case 0x212F: // TSW     - Window Area Sub Screen Disable
      case 0x2130: // CGWSEL  - Color Math Control Register A
      case 0x2131: // CGADSUB - Color Math Control Register B
        break;

      case 0x2132: // COLDATA - Color Math Sub Screen Backdrop Color
        if (value & (1 << 7)) {
          backdrop_colour.b = value & 0x1f;
        } else if (value & (1 << 6)) {
          backdrop_colour.g = value & 0x1f;
        } else if (value & (1 << 5)) {
          backdrop_colour.r = value & 0x1f;
        }
        break;

      case 0x2133: // SETINI  - Display Control 2
        setini.reg = value;
        break;

      default:;
    }
  }

  // 212c,212d
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
  } main_scr = {}, sub_scr = {};

  // 2100
  union inidisp_t {
    struct {
      byte brightness: 4;
      byte unused: 3;
      byte force_blank: 1;
    };
    byte reg;
  } inidisp {};

  // 2105
  union bgmode_t {
    struct {
      byte mode: 3;
      byte bg3_mode1_prio: 1;
      byte bg1_tile_size: 1;
      byte bg2_tile_size: 1;
      byte bg3_tile_size: 1;
      byte bg4_tile_size: 1;
    };
    byte reg;
  } bgmode {};

  // 2106
  union mosaic_t {
    struct {
      byte enable_for_bg: 4;
      byte size: 4;
    };
    byte reg;
  } mosaic {};

  // 2107-210a
  union bg_base_size_t {
    struct {
      byte sc_size: 2;
      byte base_addr: 5;
      byte base_addr_extended: 1;
    };
    byte reg;
  } bg_base_size[4] {};

  // 210b,210c (16 bits)
  union bg_char_data_addr_t {
    struct {
      byte bg1_tile_base_addr: 4;
      byte bg2_tile_base_addr: 4;
    };
    byte reg;
  } bg_char_data_addr[2] {};

  word bg_chr_base_addr_for_bg(byte bg_no) {
    switch (bg_no) {
      case 0:
        return bg_char_data_addr[0].bg1_tile_base_addr << 12;
      case 1:
        return bg_char_data_addr[0].bg2_tile_base_addr << 12;
      case 2:
        return bg_char_data_addr[1].bg1_tile_base_addr << 12;
      case 3:
        return bg_char_data_addr[1].bg2_tile_base_addr << 12;
    }
    return 0;
  }

  union vram_addr_incr_t {
    struct {
      byte step_mode: 2;
      byte addr_trans: 2; // TODO:
      byte unused: 3;
      byte after_accessing_high: 1; // Increment VRAM Address after accessing High/Low byte (0=Low, 1=High)
    };
    byte reg;
  } vram_addr_incr {};

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
  } setini {};

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
  union obsel_t {
    struct {
      byte obj_base_addr: 3;
      byte obj_gap_size: 2;
      byte obj_size: 3;
    };
    byte reg;
  } obsel {};

  union oamadd_t {
    struct {
      word addr: 10; // Values of 0x220-0x3ff are mirrors of 0x200-0x21f
      byte unused: 5;
      byte obj_prio_rotate: 1;
    };
    word reg;
  } oamadd;

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
  hvtime_t htime {};
  hvtime_t vtime {};

  enum class State {
    VISIBLE, HBLANK, VBLANK
  } state;

  union vram_addr_t {
    struct {
      byte l;
      byte h;
    };
    word w;
  } vram_addr;

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
  } windows[2] {};

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
  } bg_mask_op {}, obj_math_mask_op {};

  void dump_sprite();

  void dump_bg(byte layer);

  void dump_pal();

  byte cgram_addr {};
  bool cgram_rw_upper = false;
  byte cgram_lsb = 0;

  std::array<byte, 512 + 32> oam {};

  BusSNES* bus;

  void dump_oam(bool dump_bytes = false);

private:
  void dump_oam_table();

  void dump_oam_bytes();

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

  // vram consists of bg_map_tile_t objects (16 bits)
  std::array<dual, 0x8000> vram {};
  std::array<byte, 512> pal {};
  std::array<BGScroll, 4> scr {};

  bool oam_write_upper = false;
  byte oam_lsb = 0;

  dual vram_prefetch {};

  // Set if more than 32 sprites are on this line. Cleared at end
  // of vblank but not during fblank.
  bool sprite_range_overflow = false;

  bool hv_latched = false;
  word hloc {};
  bool hloc_read_upper = false;
  word vloc {};
  bool vloc_read_upper = false;

  Screen::colour_t backdrop_colour {};

  constexpr static byte vram_incr_step[] = {1, 32, 128, 128};
  constexpr static const char* TAG = "sppu";
  byte last_mode = 0xff;

  long ncycles {};
  long line {};
  long x {};

  std::array<SPPU::bg_map_tile_t*, 33> tiles {};
  std::array<byte, 256 + 8> row {};

  struct RenderedSprite {
    OAM oam;
    byte oam_index;
    std::vector<byte> pixels;
  };
  std::vector<RenderedSprite> visible;

  std::shared_ptr<Screen> screen;

  friend class Logger<SPPU>;

  friend class TD2;

  void render_row();

  std::array<byte, 256> render_row(byte bg);

  Screen::colour_t lookup(byte);

  std::array<word, 33> addrs_for_row(word base, word start_x, word start_y);

  word addr(word base, word x, word y, bool sx, bool sy);

  std::pair<byte, byte> get_sprite_dims(byte obsel_size, byte is_large);

  word
  obj_addr(word chr_base, word tile_no, int tile_no_x_offset, long tile_no_y_offset, long fine_y);

  void vblank_end();
};
