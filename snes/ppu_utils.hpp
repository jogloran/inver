#pragma once

#include <array>

#include "types.h"

std::array<byte, 8> decode_planar(dual* ptr, byte bpp, bool flip_x);

word compute_oam_x(OAM* oam, OAM2 *oam2, int i);

struct OAMExtras {
  word x_full;
  word tile_no_full;
  bool is_large;
};
/**
 * Computes extra OAM attributes which have to be assembled from main and secondary OAM
 * @param oam Pointer to main OAM for this sprite
 * @param oam2 Pointer to secondary OAM for this sprite
 * @param i OAM index for this sprite
 * @return OAMExtras struct
 */
OAMExtras compute_oam_extras(OAM* oam, OAM2 *oam2, int i);

/*
 * Get VRAM addresses for a whole row of BG tiles. This returns 33 tiles, since if there's
 * a fine scroll offset, it may return part of tile 0 and part of tile 32.
 * @param base The base BG tile address
 * @param start_x The leftmost tile 0 <= start_x < 64
 * @param start_y The row of the tile 0 <= start_y < 64
 * @return An array of 33 VRAM addresses for the BG tile data
 */
std::array<word, 33> addrs_for_row(word base, word start_x, word start_y);

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
word
obj_addr(word chr_base, word tile_no, int tile_no_x_offset, long tile_no_y_offset, long fine_y);

/**
 * Get (width, height) for a given OBSEL size setting
 * @param obsel_size The OBSEL size setting
 * @param is_large Whether a given sprite is large
 * @return (width, height) in pixels
 */
std::pair<byte, byte> get_sprite_dims(byte obsel_size, byte is_large);
