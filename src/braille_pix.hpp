#pragma once

#include <iterator>

#include "types.h"

/**
 * BitInserter is an output iterator which packs 64 values into 64 bits such that each
 * byte corresponds to a Unicode 8-dot Braille character representing a 4x2 bitmap.
 * Four Braille characters per row, with two rows will thus represent an 8x8 tile.
 */
class BitInserter {
public:
  std::uint64_t& output;
  int& i;

public:
  typedef byte container_type;
  typedef std::output_iterator_tag iterator_category;
  typedef void value_type;
  typedef void difference_type;
  typedef void pointer;
  typedef void reference;

  BitInserter(std::uint64_t& o, int& j) : output(o), i(j) {}

  BitInserter& operator=(byte val);

  BitInserter& operator*() { return *this; }

  BitInserter& operator++() { return *this; }

  BitInserter& operator++(int) { return *this; }
};

/* This takes a pointer with 16 valid tile pattern bytes and outputs the UTF-8 encoded
 * Braille characters corresponding to that pattern.
 */
void output_braille_sprite(byte* nt_ptr);