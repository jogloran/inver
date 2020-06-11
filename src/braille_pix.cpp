//
// Created by Daniel Tse on 11/6/20.
//

#include "braille_pix.hpp"
#include "utils.hpp"

#include <array>
#include <codecvt>
#include <locale>
#include <iostream>

std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> codec;

std::array<size_t, 64> perm {
    0, 3, 8, 11, 16, 19, 24, 27,
    1, 4, 9, 12, 17, 20, 25, 28,
    2, 5, 10, 13, 18, 21, 26, 29,
    6, 7, 14, 15, 22, 23, 30, 31,
    32, 35, 40, 43, 48, 51, 56, 59,
    33, 36, 41, 44, 49, 52, 57, 60,
    34, 37, 42, 45, 50, 53, 58, 61,
    38, 39, 46, 47, 54, 55, 62, 63
};

BitInserter& BitInserter::operator=(byte val) {
  if (val <= 1) output |= (1L << perm[i++]);
  else i++;
  return *this;
}

void output_braille_sprite(byte* nt_ptr) {
  std::uint64_t out {0};
  int j {0};
  BitInserter inserter(out, j);
  for (int i = 0; i < 8; ++i) {
    auto row = unpack_bits(nt_ptr[i], nt_ptr[i + 8]);
    std::copy(row.begin(), row.end(), inserter);
  }
  std::cout << '|';
  for (int i = 0; i < 8; ++i) {
    if (i == 4) {
      std::cout << "|\n|";
    }
    auto ch = static_cast<wchar_t>(0x2800 + (out & 0xff));
    std::cout << codec.to_bytes(ch);
    out >>= 8;
  }
  std::cout << '|' << std::endl;
}