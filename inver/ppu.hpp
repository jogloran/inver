#pragma once

#include <array>
#include <deque>

#include <gflags/gflags.h>

#include "types.h"
#include "cart.hpp"
#include "tm.hpp"
#include "screen.hpp"
#include "utils.hpp"

DECLARE_bool(xx);

class Bus;

class PPU {
public:
  using Event = std::function<void(PPU&)>;
//  enum class Event : size_t {
//    SkippedCycle,
//    NTRead, ATRead, PTReadLSB, PTReadMSB, ScrollX, ScrollY,
//    CopyX, CopyY,
//    SetVBlank,
//    ClearVBlank,
//    Shift, ExtraNTRead,
//    CalculateSprites,
//  };

  PPU() : scanline(0), ncycles(0),
          fine_x(0), w(0),
          ppudata_byte(0),
          nt_byte(0), at_byte_lsb(0), at_byte_msb(0),
          pt_byte(0), bg_tile_msb(0), bg_tile_lsb(0),
          nmi_req(false), odd_frame(false) {
    loopy_v.reg = loopy_t.reg = 0;
    tm.connect_ppu(this);
    screen.ppu = this;
    std::fill(shadow_oam.begin(), shadow_oam.end(), OAM{ 0xff, 0xff, 0xff, 0xff });
  }

  static std::array<std::function<void(PPU&)>, 14> procs;

  void events_for(int s, int c);

  void tick();

  void next_cycle();

  void load_shift_reg();

  void skip_cycle();

  void nt_read();

  void at_read();

  void pt_read_lsb();

  void pt_read_msb();

  void scx();

  void scy();

  void cpx();

  void cpy();

  void set_vblank();

  void clr_vblank();

  void shift();

  void push(Event e);

  void log_loopy() {
    log("\tcx: %02x fx: %02x cy: %02x fy: %02x\n", loopy_v.f.coarse_x, fine_x,
        loopy_v.f.coarse_y, loopy_v.f.fine_y);
  }

  void log_select(word ppu_cmd, const char* fmt, ...) {
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

  void log(const char *orig_fmt, ...) {
    if (!FLAGS_xx) {
      return;
    }

    va_list args;

    bool in_output_cycle = (ncycles >= 2 && ncycles <= 257)
                           || (ncycles >= 321 && ncycles <= 336);
    static const char *cycle_indicator[] = {
        "nt1", "nt2", "at1", "at2", "lo1", "lo2", "hi1", "++x", "", "v=t"
    };

    const char *cycle_str = "";
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
    byte lsb = cart->chr_read(plane * 4096 + tile_index * 16 + row);
    byte msb = cart->chr_read(plane * 4096 + tile_index * 16 + 8 + row);
    return unpack_bits(lsb, msb);
  }

  byte ppu_read(word ppu_addr) {
    ppu_addr &= 0x3fff;

    if (ppu_addr <= 0x1fff) {
      return cart->chr_read(ppu_addr);
    } else if (ppu_addr <= 0x3eff) {
      auto nt_index = (ppu_addr >> 10) & 3;
      auto nt_offset = ppu_addr & 0x3ff;
      return nt[nt_index * 0x400 + nt_offset];
    } else if (ppu_addr <= 0x3fff) {
      ppu_addr &= 0xff1f;
      return pal[ppu_addr - 0x3f00];
    }

    return 0;
  }

  void ppu_write(word ppu_addr, byte value) {
    ppu_addr &= 0x3fff;

    if (ppu_addr <= 0x1fff) {
      cart->chr_write(ppu_addr, value);
    } else if (ppu_addr <= 0x3eff) {
      log("nt write %02x -> %04x\n", value, ppu_addr);
      auto nt_index = (ppu_addr >> 10) & 3;
      auto nt_offset = ppu_addr & 0x3ff;
      nt[nt_index * 0x400 + nt_offset] = value;
    } else if (ppu_addr <= 0x3fff) {
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

        ppustatus.f.vblank_started = 0;
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

        loopy_v.reg += (ppuctrl.f.vram_increment ? 32 : 1);

        return result;
      }
      default:;
    }
    return 0;
  }

  void select(word ppu_cmd, byte value) {
    switch (ppu_cmd) {
      case 0x0: // ppuctrl
        ppuctrl.reg = value;
        loopy_t.f.nt_x = ppuctrl.f.nametable_base & 0x1;
        loopy_t.f.nt_y = ppuctrl.f.nametable_base & 0x2;
        break;
      case 0x1: // ppumask
        ppumask.reg = value;
        break;
      case 0x2: // ppustatus
        break;
      case 0x3: // oamaddr
        break;
      case 0x4: // oamdata
        break;
      case 0x5: // ppuscroll
        if (!w) {
          loopy_t.f.coarse_x = value >> 3;
          fine_x = value & 7;
          log("scroll cx %d fx %d cy %d fy %d\n", loopy_t.f.coarse_x, fine_x, loopy_t.f.coarse_y, loopy_t.f.fine_y);
          w = 1;
        } else {
          loopy_t.f.fine_y = value & 7;
          loopy_t.f.coarse_y = value >> 3;
          w = 0;
        }
        break;
      case 0x6: // ppuaddr
        log_select(ppu_cmd, "ppu addr write %02x\n", value);
        if (!w) {
          loopy_t.reg &= 0x3ff;
          loopy_t.reg = (loopy_t.reg & ~0x3f00) | ((value & 0x3f) << 8);
          w = 1;
        } else {
          log_select(ppu_cmd, "loopy_v before %04x\n", loopy_v.reg);
          loopy_t.reg = (loopy_t.reg & ~0xff) | value;
          loopy_v = loopy_t;
          w = 0;

          log_select(ppu_cmd, "loopy_v after %04x\n", loopy_v.reg);
          log_select(ppu_cmd, "ppu data write dest %04x\n", loopy_v.reg);
        }

        break;
      case 0x7: // ppudata
        log_select(ppu_cmd, "ppu data write %02x -> %04x\n", value, loopy_v.reg);
        ppu_write(loopy_v.reg, value);
        loopy_v.reg += (ppuctrl.f.vram_increment ? 32 : 1);
        break;
      default:;
    }
  }

  void connect(Bus *b) {
    bus = b;
  }

  void connect(std::shared_ptr<Cartridge> c) {
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

  Bus *bus;
  std::shared_ptr<Cartridge> cart;

  std::array<byte, 0x2000> pt;
  std::array<byte, 0x1000> nt;
  std::array<byte, 0x20> pal;

  union ppuctrl {
    struct {
      byte nametable_base: 2;
      byte vram_increment: 1;
      byte sprite_pattern_address: 1;
      byte background_pattern_address: 1;
      byte sprite_size: 1;
      byte ppu_master_slave: 1;
      byte vblank_nmi: 1;
    } f;
    byte reg;
  } ppuctrl;

  union ppumask {
    struct {
      byte greyscale: 1;
      byte show_left_background: 1;
      byte show_left_sprites: 1;
      byte show_background: 1;
      byte show_sprites: 1;
      byte emph_r: 1;
      byte emph_g: 1;
      byte emph_b: 1;
    } f;
    byte reg;
  } ppumask;

  union ppustatus {
    struct {
      byte junk: 5;
      byte sprite_overflow: 1;
      byte sprite0_hit: 1;
      byte vblank_started: 1;
    } f;
    byte reg;
  } ppustatus;

  struct OAM {
    byte y;
    byte tile_no;
    byte attr;
    byte x;
  };
  std::array<OAM, 64> oam;
  std::array<OAM, 8> shadow_oam;
  TM tm;
  Screen screen;
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
    } f;
    word reg;
  } loopy_v, loopy_t;
  byte fine_x;
  bool w;

  // Contains the pattern bits for the next two tiles
  word pt_shifter[2];
  // Contains the attribute bits for the next two tiles
  word at_shifter[2];

  byte nt_byte;
  byte at_byte_msb;
  byte at_byte_lsb;
  word pt_byte;
  byte bg_tile_msb;
  byte bg_tile_lsb;
  byte ppudata_byte;

  bool nmi_req;
  bool odd_frame;

  std::deque<Event> events;

  std::chrono::high_resolution_clock::time_point last_frame;

  void dump_at();

  void extra_nt_read();

  void calculate_sprites();
};
