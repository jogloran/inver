#pragma once

#include <array>
#include "types.h"
#include "logger.hpp"

#include <gflags/gflags.h>

DECLARE_bool(test_rom_output);

class SPPU : public Logger<SPPU> {
public:
  byte read(word addr) {
    switch (addr) {
      case 0x2134: // MPYL    - PPU1 Signed Multiply Result   (lower 8bit)
      case 0x2135: // MPYM    - PPU1 Signed Multiply Result   (middle 8bit)
      case 0x2136: // MPYH    - PPU1 Signed Multiply Result   (upper 8bit)
        break;

      case 0x2137: // SLHV    - PPU1 Latch H/V-Counter by Software (Read=Strobe)
        break;

      case 0x2138: // RDOAM   - PPU1 OAM Data Read            (read-twice)
        break;

      case 0x2139: // RDVRAML - PPU1 VRAM Data Read           (lower 8bits)
      case 0x213A: // RDVRAMH - PPU1 VRAM Data Read           (upper 8bits)
        break;
      case 0x213B: // RDCGRAM - PPU2 CGRAM Data Read (Palette)(read-twice)
        break;

      case 0x213C: // OPHCT   - PPU2 Horizontal Counter Latch (read-twice)
      case 0x213D: // OPVCT   - PPU2 Vertical Counter Latch   (read-twice)
      case 0x213E: // STAT77  - PPU1 Status and PPU1 Version Number
      case 0x213F: // STAT78  - PPU2 Status and PPU2 Version Number
      default:;
    }

    return 0;
  }

  void write(word addr, byte value) {
    switch (addr) {
      case 0x2100: // INIDISP - Display Control 1
        inidisp.reg = value;
        break;

      case 0x2101: // OBSEL   - Object Size and Object Base
        obsel.reg = value;
        break;

      case 0x2102: // OAMADDL - OAM Address (lower 8bit)
        oamaddl = value;
        break;

      case 0x2103: // OAMADDH - OAM Address (upper 1bit) and Priority Rotation
        oamaddh = value;
        break;

      case 0x2104: // OAMDATA - OAM Data Write (write-twice)
        break;

      case 0x2105: // BGMODE  - BG Mode and BG Character Size
        bgmode.reg = value;
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
        // How many bytes/words is the tilemap? 32*32=0x400=1024 words
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
        break;

      case 0x2115: // VMAIN   - VRAM Address Increment Mode
        vram_addr_incr.reg = value;
        break;

      case 0x2116: // VMADDL  - VRAM Address (lower 8bit)
        vram_addr.l = value;
        log("setting vram addr lo -> %04x\n", vram_addr);
        break;
      case 0x2117: // VMADDH  - VRAM Address (upper 8bit)
        vram_addr.h = value;
        log("setting vram addr hi -> %04x\n", vram_addr);
        break;

      case 0x2118: // VMDATAL - VRAM Data Write (lower 8bit)
        vram[vram_addr.w & 0x7fff].l = value;
        log("writing to %04x lo <- %02x\n", vram_addr.w & 0x7fff, value);
        if (!vram_addr_incr.after_accessing_high) {
          vram_addr.w += vram_incr_step[vram_addr_incr.step_mode];
          log("incr to %04x\n", vram_addr.w & 0x7fff);
          if (FLAGS_test_rom_output) {
            printf("\033[2J\033[H");
            for (int i = 0x7c00; i < 0x7e00; ++i) {
              bg_map_tile_t* t = (bg_map_tile_t*)&vram[i];
              if (isprint(vram[i].l) || isspace(vram[i].l))
                printf("%c", t->char_no);
              else
                printf(" ");
              if (i % 32 == 0) printf("\n");
            }
            printf("\n\n");
          }
        }
        break;
      case 0x2119: // VMDATAH - VRAM Data Write (upper 8bit)
        vram[vram_addr.w & 0x7fff].h = value;
//        printf("writing to %04x hi <- %02x\n", vram_addr.w & 0x7fff, value);
        if (vram_addr_incr.after_accessing_high) {
          vram_addr.w += vram_incr_step[vram_addr_incr.step_mode];
//          printf("incr to %04x\n", vram_addr.w & 0x7fff);
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
      case 0x2122: // CGDATA  - Palette CGRAM Data Write             (write-twice)
        break;

      case 0x2123: // W12SEL  - Window BG1/BG2 Mask Settings
      case 0x2124: // W34SEL  - Window BG3/BG4 Mask Settings
      case 0x2125: // WOBJSEL - Window OBJ/MATH Mask Settings

      case 0x2126: // WH0     - Window 1 Left Position (X1)
      case 0x2127: // WH1     - Window 1 Right Position (X2)

      case 0x2128: // WH2     - Window 2 Left Position (X1)
      case 0x2129: // WH3     - Window 2 Right Position (X2)

      case 0x212A: // WBGLOG  - Window 1/2 Mask Logic (BG1-BG4)
      case 0x212B: // WOBJLOG - Window 1/2 Mask Logic (OBJ/MATH)
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
      case 0x2132: // COLDATA - Color Math Sub Screen Backdrop Color
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
      byte bg1_mosaic_on: 1;
      byte bg2_mosaic_on: 1;
      byte bg3_mosaic_on: 1;
      byte bg4_mosaic_on: 1;
      byte mosaic_size: 4;
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

  union vram_addr_incr_t {
    struct {
      byte after_accessing_high: 2;
      byte addr_trans: 2;
      byte unused: 3;
      byte step_mode: 1; // Increment VRAM Address after accessing High/Low byte (0=Low, 1=High)
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

  byte oamaddl {};
  byte oamaddh {};

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

private:
  union dual {
    struct {
      byte l;
      byte h;
    };
    word w;
  };

  // vram consists of bg_map_tile_t objects (16 bits)
  std::array<dual, 0x8000> vram {};
  std::array<byte, 512> pal {};
  std::array<OAM, 128> oam {};
  std::array<OAM2, 32> oam2 {};

  union vram_addr_t {
    struct {
      byte l;
      byte h;
    };
    word w;
  } vram_addr;
  word cgram_addr {};

  constexpr static byte vram_incr_step[] = {1, 32, 128, 128};
  constexpr static const char* TAG = "sppu";

  friend class Logger<SPPU>;
};