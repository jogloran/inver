#pragma once

#include <array>
#include <vector>

#include <gflags/gflags.h>

#include "types.h"
#include "tm.hpp"
#include "td.hpp"
#include "screen.hpp"
#include "utils.hpp"
#include "mapper.hpp"
#include "ppu_act.hpp"
#include "shifter.hpp"

#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>

DECLARE_bool(xx);

#ifdef NDEBUG
#define LOG(msg, ...)
#else
#define LOG(msg, ...) // log(msg, __VA_ARGS__)
#endif

class Bus;

class PPU {
public:
  using Event = std::function<void(PPU&)>;

  PPU() : scanline(0), ncycles(0),
          fine_x(0), w(0),
          ppudata_byte(0),
          nt_byte(0), at_byte_lsb(0), at_byte_msb(0),
          pt_byte(0), bg_tile_msb(0), bg_tile_lsb(0),
          nmi_req(false), odd_frame(false),
          actions {
//              {at_scanline_cycle(-1, 260), log_ppu_regs}
//              {every(100, at_tile(67, Subcycle::NTRead)), log_ppu_regs},
//              {at_tile(0, -1, Subcycle::NTRead), call({log_nt_addr, decode_nt_byte, log_ppu_regs})}
          } {
    loopy_v.reg = loopy_t.reg = 0;
//    tm.connect_ppu(this);
//    td.connect_ppu(this);
    std::fill(shadow_oam.begin(), shadow_oam.end(), Sprite {{0xff, 0xff, 0xff, 0xff}, 64});
    std::fill(shadow_oam_indices.begin(), shadow_oam_indices.end(), 0xff);
    candidate_sprites.reserve(64);
  }

  void reset() {
    scanline = ncycles = fine_x = w = ppudata_byte = 0;
    nt_byte = at_byte_lsb = at_byte_msb = 0;
    pt_byte = bg_tile_msb = bg_tile_lsb = 0;
    std::fill(shadow_oam.begin(), shadow_oam.end(), Sprite {{0xff, 0xff, 0xff, 0xff}, 64});
    std::fill(shadow_oam_indices.begin(), shadow_oam_indices.end(), 0xff);
  }

  void events_for(int s, int c);

  void tick();

  void next_cycle();

  void load_shift_reg();

  void cycle_start();

  void nt_read();

  void at_read();

  void pt_read_lsb();

  void pt_read_msb();

  void scx();

  void scy();

  void cpx();

  void cpy();

  void set_vblank(bool b);

  void shift();

  void log_loopy() {
    LOG("\tcx: %02x fx: %02x cy: %02x fy: %02x\n", loopy_v.coarse_x, fine_x,
        loopy_v.coarse_y, loopy_v.fine_y);
  }

  void log_select(word ppu_cmd, const char* fmt, ...) {
    return;
    if (!FLAGS_xx) {
      return;
    }
    const char* cmds[] = {
        "ctrl", "mask", "stat", "oama", "oamd", "scrl", "addr", "data"
    };
    va_list args;
    static char buf[1024];
    std::sprintf(buf, "        <%1.1d: %4.4s> | ", ppu_cmd, cmds[ppu_cmd]);
    std::strcat(buf, fmt);
    va_start(args, fmt);
    std::vprintf(buf, args);
    va_end(args);
  }

  void log(const char* orig_fmt, ...) {
    if (!FLAGS_xx) {
      return;
    }

    va_list args;

    bool in_output_cycle = (ncycles >= 2 && ncycles <= 257)
                           || (ncycles >= 321 && ncycles <= 336);
    static const char* cycle_indicator[] = {
        "nt1", "nt2", "at1", "at2", "lo1", "lo2", "hi1", "++x", "", "v=t"
    };

    const char* cycle_str = "";
    switch (scanline) {
      case 241:
        if (ncycles == 1) {
          cycle_str = "v+";
        }
        break;
      case 261:
        if (ncycles == 1) {
          cycle_str = "v-";
        }
        break;
      case 337:
      case 338:
      case 339:
      case 340:
        cycle_str = "junk";
        break;
      default:
        size_t output_cycle_index = in_output_cycle ? (ncycles - 1) % 8 : 8;
        cycle_str = cycle_indicator[output_cycle_index];
    }
    if (ncycles == 256) {
      cycle_str = "++y";
    } else if (ncycles == 257) {
      cycle_str = "v=t";
    } else if (scanline != -1 && (ncycles >= 280 && ncycles <= 304)) {
      cycle_str = "V=T";
    }

    static char buf[1024];
    std::sprintf(buf, "[% 4d, % 4d] %4.4s | ", scanline, ncycles, cycle_str);
    std::strcat(buf, orig_fmt);
    va_start(args, orig_fmt);
    std::vprintf(buf, args);
    va_end(args);
  }

  // There are two sections, 0000-0fff and 1000-1fff, each containing 16*16=256 tiles
  // Each tile consists of 16 bytes, encoding palette values 0,1,2,3
  // The total pattern memory is 2*256*16 = 8192 bytes
  std::array<byte, 8> decode(byte plane, byte tile_index, byte row) {
    byte lsb = cart->chr_read((plane << 12) + tile_index * 16 + row);
    byte msb = cart->chr_read((plane << 12) + tile_index * 16 + 8 + row);
    return unpack_bits(lsb, msb);
  }

  byte ppu_read(word ppu_addr) {
    ppu_addr &= 0x3fff;

    if (ppu_addr <= 0x1fff) {
      return cart->chr_read(ppu_addr);
    } else if (ppu_addr <= 0x3eff) {
      // Mirroring:
      Mapper::Mirroring mirroring = cart->get_mirroring();
      switch (mirroring) {
        case Mapper::Mirroring::V:
          return nt[ppu_addr % 0x800];
        case Mapper::Mirroring::H:
          return nt[((ppu_addr / 2) & 0x400) + (ppu_addr % 0x400)];
        default:
          return nt[ppu_addr - 0x2000];
      }
    } else if (ppu_addr <= 0x3fff) {
      // Reading from palette index 0 of sprite palettes 0...3 should read from
      // index 0 of background palettes 0...3 instead
      if (ppu_addr == 0x3f10 || ppu_addr == 0x3f14 || ppu_addr == 0x3f18 || ppu_addr == 0x3f1c) {
        ppu_addr &= ~0x10;
      }
      ppu_addr &= 0xff1f;
      return pal[ppu_addr - 0x3f00] & (ppumask.greyscale ? 0x30 : 0xff);
    }

    return 0;
  }

  void ppu_write(word ppu_addr, byte value) {
    ppu_addr &= 0x3fff;

    if (ppu_addr <= 0x1fff) {
      cart->chr_write(ppu_addr, value);
    } else if (ppu_addr <= 0x3eff) {
      // Mirroring:
      Mapper::Mirroring mirroring = cart->get_mirroring();
      switch (mirroring) {
        case Mapper::Mirroring::V:
          nt[ppu_addr % 0x800] = value;
          break;
        case Mapper::Mirroring::H:
          nt[((ppu_addr / 2) & 0x400) + (ppu_addr % 0x400)] = value;
          break;
        default:
          nt[ppu_addr - 0x2000] = value;
          break;
      }
    } else if (ppu_addr <= 0x3fff) {
      // Writing to palette index 0 of sprite palettes 0...3 should write to
      // index 0 of background palettes 0...3 instead
      if (ppu_addr == 0x3f10 || ppu_addr == 0x3f14 || ppu_addr == 0x3f18 || ppu_addr == 0x3f1c) {
        ppu_addr &= ~0x10;
      }
      // 3f01 == 3f21
      // 00111111 0000 0001
      // 00111111 0010 0001
      ppu_addr &= 0xff1f;
      pal[ppu_addr - 0x3f00] = value;
    }
  }

  byte select(word ppu_cmd) {
    switch (ppu_cmd) {
      case 0x0: // ppuctrl
        break;
      case 0x1: // ppumask
        break;
      case 0x2: { // ppustatus
        byte val = (ppustatus.reg & 0xe0);
        w = 0;

        ppustatus.vblank_started = 0;
        return val;
      }
      case 0x3: // oamaddr
        break;
      case 0x4: // oamdata
        break;
      case 0x5: // ppuscroll
        break;
      case 0x6: // ppuaddr
        break;
      case 0x7: { // ppudata
        byte result = ppudata_byte;

        ppudata_byte = ppu_read(loopy_v.reg);
        if (loopy_v.reg >= 0x3f00) result = ppudata_byte;

        loopy_v.reg += (ppuctrl.vram_increment ? 32 : 1);

        return result;
      }
      default:;
    }
    return 0;
  }

  void select(word ppu_cmd, byte value);

  void connect(Bus* b) {
    bus = b;
  }

  void connect(std::shared_ptr<Mapper> c) {
    cart = c;
  }

  void dump_nt() {
    for (int row = 0; row < 30; ++row) {
      for (int col = 0; col < 32; ++col) {
        std::printf("%02x ", ppu_read(0x2000 + row * 32 + col));
      }
      std::printf("\n");
    }
    std::printf("\n");
  }

  void dump_pt() {
    std::printf("bg: %02x\n", pal[0]);
    for (int i = 0; i < 8; ++i) {
      std::printf("% 2d: %02x %02x %02x %02x\n", i % 4,
                  pal[4 * i], pal[4 * i + 1],
                  pal[4 * i + 2], pal[4 * i + 3]);
    }
  }

  Bus* bus;
  std::shared_ptr<Mapper> cart;

  std::array<byte, 0x800> nt;
  std::array<byte, 0x20> pal;

  std::vector<PPULogSpec> actions;

  union ppuctrl {
    struct {
      byte nt_base: 2;
      byte vram_increment: 1;
      byte sprite_pt_addr: 1;
      byte bg_pt_addr: 1;
      byte sprite_size: 1;
      byte ppu_master_slave: 1;
      byte vblank_nmi: 1;
    };
    byte reg;

    template <typename Ar>
    void serialize(Ar& ar) { ar(reg); }
  } ppuctrl;

  union ppumask {
    struct {
      byte greyscale: 1;
      byte render_left_bg: 1;
      byte render_left_sprites: 1;
      byte render_bg: 1;
      byte render_sprites: 1;
      byte emph_r: 1;
      byte emph_g: 1;
      byte emph_b: 1;
    };
    byte reg;

    template <typename Ar>
    void serialize(Ar& ar) { ar(reg); }
  } ppumask;

  union ppustatus {
    struct {
      byte junk: 5;
      byte sprite_overflow: 1;
      byte sprite0_hit: 1;
      byte vblank_started: 1;
    };
    byte reg;

    template <typename Ar>
    void serialize(Ar& ar) { ar(reg); }
  } ppustatus;

  struct OAM {
    byte y;
    byte tile_no;
    byte attr;
    byte x;

    template <typename Ar>
    void serialize(Ar& ar) { ar(y, tile_no, attr, x); }
  };
  struct Sprite {
    PPU::OAM oam;
    byte sprite_index;

    template <typename Ar>
    void serialize(Ar& ar) { ar(oam, sprite_index); }
  };
  std::array<OAM, 64> oam;
  std::array<Sprite, 8> shadow_oam;
  std::array<byte, 8> shadow_oam_indices;
  std::vector<Sprite> candidate_sprites;
  std::array<byte, 256> sprite_row;
  TM tm;
  TD td;
  std::shared_ptr<Screen> screen;
  int scanline; // -1 to 260
  int ncycles;

  union {
    struct {
      word coarse_x: 5;
      word coarse_y: 5;
      byte nt_x: 1;
      byte nt_y: 1;
      word fine_y: 3;
      byte unused: 1;
    };
    word reg;

    template <typename Ar>
    void serialize(Ar& ar) { ar(reg); }
  } loopy_v, loopy_t;
  byte fine_x;
  bool w;

  // Contains the pattern bits for the next two tiles
  Shifter pt;
  // Contains the attribute bits for the next two tiles
  Shifter at;

  byte nt_byte;
  byte at_byte_msb;
  byte at_byte_lsb;
  word pt_byte;
  byte bg_tile_msb;
  byte bg_tile_lsb;
  byte ppudata_byte;

  bool nmi_req;
  bool odd_frame;

  std::chrono::high_resolution_clock::time_point frame_start;

  std::array<bool, 256> bg_is_transparent;

  void dump_at();

  void dump_oam();

  void extra_nt_read();

  void calculate_sprites();

  void frame_done();

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(nt, pal, ppuctrl, ppumask, ppustatus, oam, shadow_oam, shadow_oam_indices, candidate_sprites, sprite_row, scanline, ncycles, loopy_v, loopy_t, fine_x, w, pt, at, nt_byte, at_byte_msb, at_byte_lsb, pt_byte, bg_tile_msb, bg_tile_lsb, ppudata_byte, nmi_req, odd_frame, bg_is_transparent);
  }

  bool rendering() const;
};
