#pragma once

#include <array>

#include "regs.hpp"
#include "types.h"

/**
 * Decodes 1, 2 or 4 words of m7_chr data representing one row of 8 tile pixels
 * into an array of 8 palette indices.
 * @param ptr The initial offset of the m7_chr data
 * @param wpp Words per pixel (1, 2 or 4)
 * @param flip_x Whether the tile is horizontally flipped
 * @return An array of 8 palette indices
 */
std::array<byte, 8> decode_planar(const dual* ptr, byte wpp, bool flip_x);

/**
 * (legacy) Compute only the extended 9-bit x offset from OAM and extended OAM.
 */
word compute_oam_x(const OAM* oam, const OAM2* oam2, int i);

struct OAMExtras {
  word x_full;       // The full 9-bit x offset
  word tile_no_full; // The full 9-bit tile number
  bool is_large;
};

/**
 * Computes extra OAM attributes which have to be assembled from main and secondary OAM
 * @param oam Pointer to main OAM for this sprite
 * @param oam2 Pointer to secondary OAM for this sprite
 * @param i OAM index for this sprite
 * @return OAMExtras struct
 */
OAMExtras compute_oam_extras(const OAM* oam, const OAM2* oam2, int i);

/**
 * Compute the VRAM address for a BG tile.
 * @param base The base BG tile address
 * @param x 0 <= x < 64
 * @param y 0 <= y < 64
 * @param sx Whether we are in the right half of the 64x64 tile space
 * @param sy Whether we are in the bottom half of the 64x64 tile space
 * @return 16-bit VRAM address
 */
word addr(word base, word x, word y, bool sx, bool sy);

/**
 * Compute the VRAM address for a sprite tile.
 * @param chr_base The base sprite tile address
 * @param tile_no 0 <= tile_no < 512
 * @param tile_no_x_offset
 * @param tile_no_y_offset
 * @param fine_y 0 <= fine_y < 8
 * @return 16-bit VRAM address
 */
word obj_addr(word chr_base, word tile_no, int tile_no_x_offset, long tile_no_y_offset, long fine_y);

/**
 * Compute the VRAM address for BG tile m7_chr data.
 * @param chr_base The base BG m7_chr tile address
 * @param tile_id 0 <= tile_no < 256
 * @param fine_y 0 <= fine_y < 8
 * @param wpp Words per pixel (bpp / 2)
 * @return 16-bit VRAM address
 */
word tile_chr_addr(word chr_base, word tile_id, byte fine_y, byte wpp);

/**
 * Get (width, height) for a given OBSEL size setting
 * @param obsel_size The OBSEL size setting
 * @param is_large Whether a given sprite is large
 * @return (width, height) in pixels
 */
std::pair<byte, byte> get_sprite_dims(byte obsel_size, byte is_large);

/**
 * A reduce which allows the operation to decide when to terminate the reduction
 * early.
 * @param op A callable taking (_Tp, _Tp, bool&), where the third argument can
 * be assigned _true_ if early termination is desired.
 */
template <class _InputIterator, class _Tp, class _Arg3>
_Tp reduce3(_InputIterator first, _InputIterator last, _Tp init, _Arg3 op) {
  bool should_stop = false;
  for (; first != last; ++first) {
    init = op(init, *first, should_stop);
    if (should_stop) break;
  }
  return init;
}