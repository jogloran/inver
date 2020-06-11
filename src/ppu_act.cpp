#include "ppu_act.hpp"
#include "ppu.hpp"
#include "braille_pix.hpp"

Pred at_line(int scanline) {
  return [scanline](PPU& ppu) {
    return scanline == ppu.scanline;
  };
}

bool subcycle_matches(const Subcycle& s, int ncycles);

Pred at_tile(int tile_col, int tile_row, Subcycle s) {
  if (tile_col < 0) tile_col = 30 + tile_col;
  if (tile_row < 0) tile_row = 32 + tile_row;
  return at_tile(tile_col * 32 + tile_row, s);
}

Pred at_tile(int tile_no, Subcycle s) {
  return [=](PPU& ppu) {
    if ((tile_no % 32) <= 1) {
      if (ppu.scanline == tile_no / 4 - 1 &&
          ppu.ncycles >= 8 * (tile_no % 32) + 321 &&
          ppu.ncycles < 8 * (tile_no % 32) + 329) {
        return subcycle_matches(s, ppu.ncycles);
      }
    } else if (ppu.scanline == tile_no / 4 &&
               ppu.ncycles >= 8 * (tile_no % 32) - 15 &&
               ppu.ncycles < 8 * (tile_no % 32) - 7) {
      return subcycle_matches(s, ppu.ncycles);
    }
    return false;
  };
}

inline bool subcycle_matches(const Subcycle& s, int ncycles) {
  return s == Subcycle::All ||
         (ncycles - 1) % 8 == static_cast<int>(s);
}

Pred at_scanline_cycle(int scanline, int cycle) {
  return [scanline, cycle](PPU& ppu) {
    return ppu.scanline == scanline && ppu.ncycles == cycle;
  };
}

Pred every(size_t n, Pred inner) {
  size_t i = 0;
  return [i, n, inner](PPU& ppu) mutable {
    if (inner(ppu)) {
      if (i++ % n == 0) {
        return true;
      }
    }
    return false;
  };
}

Pred first(Pred inner) {
  bool hit = false;
  return [hit, inner](PPU& ppu) mutable {
    if (inner(ppu)) {
      if (!hit) {
        hit = true;
        return true;
      }
    }
    return false;
  };
}

Act call(std::vector<Act> acts) {
  return [acts](PPU& ppu) {
    for (Act a : acts) {
      a(ppu);
    }
  };
}

void log_ppu_regs(PPU& ppu) {
  ppu.log("nt: %02x at: %02x pt: %02x%02x\n", ppu.nt_byte, ppu.at_byte_lsb, ppu.at_byte_msb,
          ppu.pt_byte);
}

void decode_nt_byte(PPU& ppu) {
  std::array<byte, 16> sprite_bytes;
  for (int i = 0; i < 16; ++i) {
    sprite_bytes[i] = ppu.cart->chr_read(
        (ppu.ppuctrl.background_pattern_address << 12) + ppu.nt_byte * 16 + i);
  }
  output_braille_sprite(sprite_bytes.data());
}