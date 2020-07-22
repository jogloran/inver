#pragma once

#include <array>

#include "types.h"

// Decode planar data
// Takes 2bpp, 4bpp or 8bpp data (1 word, 2 words or 4 words)
// produces an array of 8 palette indices decoded from the planar data

std::array<byte, 8> decode(word w, bool flip_x) {
  std::array<byte, 8> result;
  byte lsb = w & 0xff;
  byte msb = w >> 8;
  for (int i = 0; i < 8; ++i) {
    auto bit_select = flip_x ? i : 7 - i;
    result[7 - i] = (((msb & (1 << bit_select)) != 0) << 1)
                    + ((lsb & (1 << bit_select)) != 0);
  }
  return result;
}

std::array<byte, 8> decode(word w1, word w2, bool flip_x) {
  std::array<byte, 8> result;
  byte b1 = w1 & 0xff;
  byte b2 = w1 >> 8;
  byte b3 = w2 & 0xff;
  byte b4 = w2 >> 8;
  for (int i = 0; i < 8; ++i) {
    auto bit_select = flip_x ? i : 7 - i;
    result[7 - i] =
        (((b4 & (1 << bit_select)) != 0) << 4)
        + (((b3 & (1 << bit_select)) != 0) << 2)
        + (((b2 & (1
            << bit_select)) != 0) << 1)
        + (b1 & (1 << bit_select)) != 0;
  }
  return result;
}

std::array<byte, 8> decode_planar(dual* ptr, byte bpp, bool flip_x) {
  if (bpp == 1) {
    return decode((*ptr).w, flip_x);
  } else {
    return decode((*ptr).w, (*(ptr + 1)).w, flip_x);
  }
}