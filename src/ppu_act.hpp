#pragma once

#include <functional>

class PPU;

using Pred = std::function<bool(PPU&)>;
using Act  = std::function<void(PPU&)>;

enum class Subcycle : int {
  NTRead = 0,
  ATRead = 2,
  PTReadLSB = 4,
  PTReadMSB = 6,
  ScrollX = 7,
  All = -1,
};

Pred at_tile(int tile_col, int tile_row, Subcycle s = Subcycle::All);

Pred at_tile(int tile_no, Subcycle s = Subcycle::All);

Pred at_scanline_cycle(int scanline, int cycle);

void log_ppu_regs(PPU& ppu);

void decode_nt_byte(PPU& ppu);

Pred every(size_t n, Pred inner);
Pred first(Pred inner);

struct PPULogSpec {
  Pred pred;
  Act action;
};
