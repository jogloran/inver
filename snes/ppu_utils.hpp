#pragma once

#include <array>

#include "types.h"

std::array<byte, 8> decode(word w, bool flip_x);

std::array<byte, 8> decode(word w1, word w2, bool flip_x);

std::array<byte, 8> decode_planar(dual* ptr, byte bpp, bool flip_x);

word compute_oam_x(SPPU::OAM* oam, SPPU::OAM2 *oam2, int i);

struct OAMExtras {
  word x_full;
  word tile_no_full;
  bool is_large;
};
OAMExtras compute_oam_extras(SPPU::OAM* oam, SPPU::OAM2 *oam2, int i);