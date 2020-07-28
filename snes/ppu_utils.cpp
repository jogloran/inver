#include <array>

#include "types.h"
#include "sppu.hpp"
#include "ppu_utils.hpp"

// Decode planar data
// Takes 2bpp, 4bpp or 8bpp data (1 word, 2 words or 4 words)
// produces an array of 8 palette indices decoded from the planar data

std::array<byte, 8> decode(word w, bool flip_x) {
  std::array<byte, 8> result;
  byte lsb = w & 0xff;
  byte msb = w >> 8;
  for (int i = 0; i < 8; ++i) {
    auto bit_select = flip_x ? i : 7 - i;
    result[i] = !!(lsb & (1 << bit_select)) + 2 * !!(msb & (1 << bit_select));
  }
  return result;
}

std::array<byte, 8> decode(word w1, word w2, bool flip_x) {
  std::array<byte, 8> result;
  auto plane1 = decode(w1, flip_x);
  auto plane2 = decode(w2, flip_x);
  for (int i = 0; i < 8; ++i) {
    result[i] = plane1[i] + (plane2[i] << 2);
  }
  return result;
}

std::array<byte, 8> decode_planar(dual* ptr, byte bpp, bool flip_x) {
  if (bpp == 1) {
    return decode((*ptr).w, flip_x);
  } else {
    return decode((*ptr).w, (*(ptr + 8)).w, flip_x);
  }
}

word compute_oam_x(SPPU::OAM* oam, SPPU::OAM2 *oam2, int i) {
  switch (i % 4) {
    case 0:
      return oam->x + (oam2->obj0_xh << 8);
    case 1:
      return oam->x + (oam2->obj1_xh << 8);
    case 2:
      return oam->x + (oam2->obj2_xh << 8);
    case 3:
      return oam->x + (oam2->obj3_xh << 8);
  }
  return 0;
}

OAMExtras compute_oam_extras(SPPU::OAM* oam, SPPU::OAM2* oam2, int i) {
  auto full_tile_no = static_cast<word>(oam->tile_no | (oam->attr.tile_no_h << 8));

  switch (i % 4) {
    case 0:
      return { static_cast<word>(oam->x + (oam2->obj0_xh << 8)),
               full_tile_no,               bool(oam2->obj0_sz) };
    case 1:
      return { static_cast<word>(oam->x + (oam2->obj1_xh << 8)),
               full_tile_no,               bool(oam2->obj1_sz) };
    case 2:
      return { static_cast<word>(oam->x + (oam2->obj2_xh << 8)),
               full_tile_no,               bool(oam2->obj2_sz) };
    case 3:
      return { static_cast<word>(oam->x + (oam2->obj3_xh << 8)),
               full_tile_no,               bool(oam2->obj3_sz) };
  }
  return {};
}