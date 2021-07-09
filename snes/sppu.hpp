#pragma once

#include <array>

#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>

#include "logger.hpp"
#include "regs.hpp"
#include "screen.hpp"
#include "types.h"

#include <gflags/gflags.h>

DECLARE_bool(test_rom_output);

class BusSNES;

class Screen;

class Layers;

struct LayerSpec {
  byte layer;
  byte prio;
};

class SPPU: public Logger<SPPU> {
public:
  void connect(BusSNES* b) {
    bus = b;
  }

  void connect(Screen* s) {
    screen = s;
  }

  void reset();

  void tick(byte master_cycles = 1);

  bool is_pal_clear(byte p) const { return p % 16 == 0; }

  byte read(word addr) {
    switch (addr) {
      case 0x2134: // MPYL    - PPU1 Signed Multiply Result   (lower 8bit)
        return mpyx.l;
      case 0x2135: // MPYM    - PPU1 Signed Multiply Result   (middle 8bit)
        return mpyx.m;
      case 0x2136: // MPYH    - PPU1 Signed Multiply Result   (upper 8bit)
        return mpyx.h;

      case 0x2137: // SLHV    - PPU1 Latch H/V-Counter by Software (Read=Strobe)
        hv_latched = true;
        hloc.w = x;
        vloc.w = line;
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
        if (!hloc_read_upper) {
          hloc_read_upper = true;
          return hloc.l;
        } else {
          hloc_read_upper = false;
          return hloc.h;
        }
      case 0x213D: // OPVCT   - PPU2 Vertical Counter Latch   (read-twice)
        if (!vloc_read_upper) {
          vloc_read_upper = true;
          return vloc.l;
        } else {
          vloc_read_upper = false;
          return vloc.h;
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
        if (addr == 0x210d || addr == 0x210e) {
          m7.set(addr, value);
        }

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
        if (!vram_addr_incr.after_accessing_high) {
          vram_addr.w += vram_incr_step[vram_addr_incr.step_mode];
        }
        break;
      case 0x2119: // VMDATAH - VRAM Data Write (upper 8bit)
        vram[vram_addr.w & 0x7fff].h = value;
        if (vram_addr_incr.after_accessing_high) {
          vram_addr.w += vram_incr_step[vram_addr_incr.step_mode];
        }
        break;

      case 0x211A: // M7SEL   - Rotation/Scaling Mode Settings
        m7sel.reg = value;
        log_with_tag("m7", "m7sel nowrap %d fill0 %d flip v %d h %d\n", m7sel.no_wrap, m7sel.fill_with_tile_0, m7sel.screen_vflip, m7sel.screen_hflip);
        break;

      case 0x211B: // M7A     - Rotation/Scaling Parameter A & Maths 16bit operand
      case 0x211C: // M7B     - Rotation/Scaling Parameter B & Maths 8bit operand
      case 0x211D: // M7C     - Rotation/Scaling Parameter C         (write-twice)
      case 0x211E: // M7D     - Rotation/Scaling Parameter D         (write-twice)
      case 0x211F: // M7X     - Rotation/Scaling Center Coordinate X (write-twice)
      case 0x2120: // M7Y     - Rotation/Scaling Center Coordinate Y (write-twice)
        log_with_tag("m7", "p %04x <- %02x\n", addr, value);
        m7.set(addr, value);
        if (addr == 0x211c) mpyx.w = m7.a() * m7.b();
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
        log_with_tag("win", "line %d win %d l pos %02x\n", line, 0, value);
        windows[0].l = value;
        break;
      case 0x2127: // WH1     - Window 1 Right Position (X2)
        log_with_tag("win", "line %d win %d r pos %02x\n", line, 0, value);
        windows[0].r = value;
        break;

      case 0x2128: // WH2     - Window 2 Left Position (X1)
        log_with_tag("win", "line %d win %d l pos %02x\n", line, 1, value);
        windows[1].l = value;
        break;
      case 0x2129: // WH3     - Window 2 Right Position (X2)
        log_with_tag("win", "line %d win %d r pos %02x\n", line, 1, value);
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
        window_main_disable_mask.reg = value;
        break;
      case 0x212F: // TSW     - Window Area Sub Screen Disable
        window_sub_disable_mask.reg = value;
        break;

      case 0x2130: // CGWSEL  - Color Math Control Register A
        cgwsel.reg = value;
        break;
      case 0x2131: // CGADSUB - Color Math Control Register B
        colour_math.reg = value;
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

  enum State {
    VISIBLE = 0,
    HBLANK = 1,
    VBLANK = 2,
    HBLANK_IN_VBLANK = 3,
  } state;

  // region PPU registers

  // 212c,212d
  layer_ctrl_t main_scr = {}, sub_scr = {};
  // 2100
  inidisp_t inidisp {};
  // 2105
  bgmode_t bgmode {};
  // 2106
  mosaic_t mosaic {};
  // 2107-210a
  bg_base_size_t bg_base_size[4] {};

  // 210b,210c (16 bits)
  bg_char_data_addr_t bg_char_data_addr[2] {};

  vram_addr_incr_t vram_addr_incr {};
  setini_t setini {};
  obsel_t obsel {};
  oamadd_t oamadd {};

  hvtime_t htime {};
  hvtime_t vtime {};

  vram_addr_t vram_addr;

  window_t windows[2] {};
  window_mask_op_t bg_mask_op {}, obj_math_mask_op {};
  window_disable_mask_t window_main_disable_mask {}, window_sub_disable_mask {};

  cgwsel_t cgwsel {};
  cgadsub_t colour_math {};

  byte cgram_addr {};
  bool cgram_rw_upper = false;
  byte cgram_lsb = 0;

  m7sel_t m7sel {};

  // endregion

  BusSNES* bus;

  Screen* screen;

  /**
   * Render a single row of background screen output.
   * @param bg The layer to render (0, 1, 2, 3)
   * @param prio 0,1,2,3
   * @return An array of palette values for each pixel in this row
   */
  std::array<byte, 256> render_row(byte bg, byte prio) const;

  /**
   * Render a single row of sprite output.
   * @param prio 0,1,2,3
   * @return An array of palette values for each pixel in this row
   */
  std::array<byte, 256> render_obj(byte prio);

  /**
   * Compute the mask row for a given background layer.
   */
  std::array<byte, 256> compute_mask(byte layer) const;

  M7Params m7 {};

  long line {};

private:
  std::array<byte, 512 + 32> oam {};

  // vram consists of bg_map_tile_t objects (16 bits)
  std::array<dual, 0x8000> vram {};
  std::array<byte, 512> pal {};
  std::array<BGScroll, 4> scr {};
  std::array<word, 256> main {};
  std::array<word, 256> sub {};

  std::vector<byte> pixels {};

  byte oam_lsb = 0;

  dual vram_prefetch {};

  // Set if more than 32 sprites are on this line. Cleared at end
  // of vblank but not during fblank.
  bool sprite_range_overflow = false;

  bool hv_latched = false;
  dual hloc {};
  bool hloc_read_upper = false;
  dual vloc {};
  bool vloc_read_upper = false;

  colour_t backdrop_colour {};

  constexpr static byte vram_incr_step[] = {1, 32, 128, 128};
  constexpr static const char* TAG = "sppu";
  byte last_mode = 0xff; // last mode set, for debugging purposes

  long ncycles {};
  long x {};

  // region PPU caches
  mutable std::array<byte, 256 + 8> row {};
  mutable std::array<colour_t, 256> pals {};
  mutable std::array<word, 33> addrs {};

  std::pair<std::vector<LayerSpec>, std::vector<LayerSpec>> main_sub {};

  s24_t mpyx {};

  struct RenderedSprite {
    OAM oam;
    byte oam_index;
    std::vector<byte> pixels;

    template<typename Ar>
    void serialize(Ar& ar) {
      ar(oam, oam_index, pixels);
    }
  };

  std::vector<RenderedSprite> visible;
  // endregion

  friend class Logger<SPPU>;

  friend class TD2;

  friend class M7;

  friend class CPU5A22;

  friend class BusSNES;

  /**
   * Finds the first opaque pixel given the layer specs at a given
   * horizontal position.
   * @returns { layer index, pal, is_masked }
   */
  auto prio_sort(const std::vector<LayerSpec>& layers, const Layers& l, int i) const;

  /**
   * Computes the pixel output for the current row, and blits it to the frame buffer.
   */
  void render_row();

  /**
   * Given the contents of CGRAM, converts a palette index into an RGB colour value.
   */
  colour_t lookup(byte) const;

  /**
   * Signals the end of the vblank period.
   */
  void vblank_end();

  /*
 * Get VRAM addresses for a whole row of BG tiles. This returns 33 tiles, since if there's
 * a fine scroll offset, it may return part of tile 0 and part of tile 32.
 * @param base The base BG tile address
 * @param start_x The leftmost tile 0 <= start_x < 64
 * @param start_y The row of the tile 0 <= start_y < 64
 * @param sc_size The mirroring mode from BGxSC 0 <= sc_size < 4
 * @return An array of 33 VRAM addresses for the BG tile data
 */
  void compute_addrs_for_row(word base, word start_x, word start_y, byte sc_size) const;

  /**
   * Get the appropriate row of palette indices from the layer data
   * @param l Layer data
   * @param layer Layer index
   * @param prio Priority (0,1,2,3)
   * @return Reference to palette indices
   */
  static const std::array<byte, 256>& get_pal_row(const Layers& l, byte layer, byte prio);

  /**
   * Get the appropriate window row mask from the layer data
   * @param l Layer data
   * @param layer Layer index
   * @param prio Priority (0,1,2,3)
   * @return Reference to window row mask
   */
  static const std::array<byte, 256>& get_mask_row(const Layers& l, byte layer);

  /**
   * Given a the layer specs for this mode and the current PPU state,
   * partitions the layer specs into MAIN and SUB.
   * @param prios Layer specs
   */
  auto& route_main_sub(const std::vector<LayerSpec>& prios);

  /**
   * Given the current PPU state and a horizontal position _i_,
   * decides whether colour math should apply at this pixel.
   */
  bool colour_math_applies(int i, const Layers& layers) const;

  /**
   * Computes the base address for background tile character data.
   * @param bg_no 0 <= bg_no < 4, the layer to compute the base address for
   */
  word bg_chr_base_addr_for_bg(byte bg_no) const {
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

  std::array<byte, 256> main_source_layer {};
  std::array<byte, 256> sub_source_layer {};

  static word PAL_MASKED_IN_WINDOW;
  static word PAL_SUB_SCREEN_TRANSPARENT;

  friend class Screen;

  friend class PPUDebug;

  /**
   * Returns adjusted positions after accounting for x- and y-scrolling and the mosaic effect.
   */
  auto get_tile_pos(byte bg) const;

  /**
   * Blits the current line of composed RGB data to the corresponding line of the display.
   */
  void blit();

public:
  template<typename Ar>
  void serialize(Ar& ar) {
    ar(main_scr, sub_scr,
       inidisp, bgmode, mosaic, bg_base_size,
       bg_char_data_addr,
       vram_addr_incr, setini, obsel, oamadd,
       htime, vtime, vram_addr,
       windows, window_main_disable_mask, window_sub_disable_mask,
       bg_mask_op, obj_math_mask_op,
       cgwsel, colour_math,
       cgram_addr, cgram_rw_upper, cgram_lsb,
       oam, vram, pal, scr, oam_lsb, vram_prefetch,
       sprite_range_overflow,
       hv_latched, hloc, hloc_read_upper, vloc, vloc_read_upper,
       backdrop_colour, last_mode,
       ncycles, line, x,
       visible, m7, m7sel);
  }

  /**
   * Alternate rendering pipeline for mode 7.
   */
  std::array<byte, 256> render_row_mode7(int bg);
  void fill_dummy_vram();
};
