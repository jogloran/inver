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

std::array<byte, 8> decode(word w1, word w2, word w3, word w4, bool flip_x) {
  std::array<byte, 8> result;
  auto plane1 = decode(w1, w2, flip_x);
  auto plane2 = decode(w3, w4, flip_x);
  for (int i = 0; i < 8; ++i) {
    result[i] = plane1[i] + (plane2[i] << 8);
  }
  return result;
}

std::array<byte, 8> decode_planar(dual* ptr, byte bpp, bool flip_x) {
  switch (bpp) {
    case 1:
      return decode((*ptr).w, flip_x);
    case 2:
      return decode((*ptr).w, (*(ptr + 8)).w, flip_x);
    case 4:
      return decode((*ptr).w, (*(ptr + 8)).w,
                    (*(ptr + 16)).w, (*(ptr + 24)).w, flip_x);
  }
  assert("invalid bpp");
  return {};
}

word compute_oam_x(OAM* oam, OAM2* oam2, int i) {
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

OAMExtras compute_oam_extras(OAM* oam, OAM2* oam2, int i) {
  auto full_tile_no = static_cast<word>(oam->tile_no | (oam->attr.tile_no_h << 8));

  switch (i % 4) {
    case 0:
      return {static_cast<word>(oam->x + (oam2->obj0_xh << 8)),
              full_tile_no, bool(oam2->obj0_sz)};
    case 1:
      return {static_cast<word>(oam->x + (oam2->obj1_xh << 8)),
              full_tile_no, bool(oam2->obj1_sz)};
    case 2:
      return {static_cast<word>(oam->x + (oam2->obj2_xh << 8)),
              full_tile_no, bool(oam2->obj2_sz)};
    case 3:
      return {static_cast<word>(oam->x + (oam2->obj3_xh << 8)),
              full_tile_no, bool(oam2->obj3_sz)};
  }
  return {};
}

std::array<word, 33> addrs_for_row(word base, word start_x, word start_y,
    byte sc_size) {
  //                sx,    sy
  // sc_size = 0 => false, false
  // sc_size = 1 => true, false
  // sc_size = 2 => false, true
  // sc_size = 3 => true, true
  auto sx = sc_size & 1;
  auto sy = sc_size >> 1;

  std::array<word, 33> result;
  auto it = result.begin();
  for (int i = 0; i < 33; ++i) {
    *it++ = addr(base, (start_x + i) % 64, start_y, sx, sy);
  }
  return result;
}

word addr(word base, word x, word y, bool sx, bool sy) {
  return (base
          + ((y & 0x1f) << 5)
          + (x & 0x1f)
          + (sy ? ((y & 0x20) << (sx ? 6 : 5)) : 0)
          + (sx ? ((x & 0x20) << 5) : 0));
}

std::pair<byte, byte> get_sprite_dims(byte obsel_size, byte is_large) {
  static constexpr std::array<std::pair<byte, byte>, 16> table
      {{
           {8, 8}, {8, 8}, {8, 8}, {16, 16}, {16, 16}, {32, 32}, {16, 32}, {16, 32},
           {16, 16}, {32, 32}, {64, 64}, {32, 32}, {64, 64}, {64, 64}, {32, 64}, {32, 32},
       }};
  return table[(is_large ? 8 : 0) + obsel_size];
}

word obj_addr(word chr_base, word tile_no, int tile_no_x_offset, long tile_no_y_offset,
              long fine_y) {
  return chr_base
         + ((tile_no + tile_no_x_offset) << 4) // 8x8 tile row selector
         + (tile_no_y_offset << 8) // 8x8 tile column selector
         + fine_y;
}

word tile_chr_addr(word chr_base, word tile_id, byte fine_y, byte wpp) {
  return chr_base + (8 * wpp) * tile_id + fine_y;
}