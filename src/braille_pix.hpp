#pragma once

#include <iterator>

#include "types.h"

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

void output_braille_sprite(byte* nt_ptr);