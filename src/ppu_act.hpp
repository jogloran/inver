#pragma once

#include <functional>
#include <vector>

class PPU;

// a Pred decides when an action may fire
using Pred = std::function<bool(PPU&)>;
// an Act implements the action
using Act  = std::function<void(PPU&)>;

struct PPULogSpec {
  Pred pred;
  Act action;
};

enum class Subcycle : int {
  NTRead = 0,
  ATRead = 2,
  PTReadLSB = 4,
  PTReadMSB = 6,
  ScrollX = 7,
  All = -1,
};

// Preds

/**
 * Triggers at tile (tile_col, tile_row) at a certain subcycle.
 */
Pred at_tile(int tile_col, int tile_row, Subcycle s = Subcycle::All);

/**
 * Triggers at tile tile_no (0 <= tile_no < 959) at a certain subcycle.
 */
Pred at_tile(int tile_no, Subcycle s = Subcycle::All);

/**
 * Triggers at a given scanline (-1 <= scanline <= 260) and cycle (0 <= cycle <= 340).
 */
Pred at_scanline_cycle(int scanline, int cycle);

/**
 * Triggers every n times the inner Pred triggers.
 */
Pred every(size_t n, Pred inner);

/**
 * Triggers the first time the inner Pred triggers.
 */
Pred first(Pred inner);

// Acts
/**
 * Calls each of a sequence of sub-actions in turn.
 */
Act call(std::vector<Act> acts);
void log_nt_addr(PPU& ppu);
void log_ppu_regs(PPU& ppu);
void decode_nt_byte(PPU& ppu);