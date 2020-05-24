#pragma once

#include "ppu_base.h"
#include "types.h"
#include "util.h"

#include <algorithm>

void
compare_oams(OAM* current_oam, OAM* old_oam);

template <typename T, unsigned long A, unsigned long B> void
flatten(const std::array<std::array<T, A>, B>& in, typename std::array<T, A*B>::iterator begin) {
  auto ptr = begin;
  for (auto it = in.begin(); it != in.end(); ++it) {
    ptr = std::copy(it->begin(), it->end(), ptr);
  }
}

template <typename InIter, typename OutIter>
void rotate_tiles(byte offset, InIter src, OutIter dest) {
  auto n_copied_from_end = std::min(21, 32 - offset);
  auto cur = std::copy_n(src + offset, n_copied_from_end, dest);
  std::copy_n(src, 21 - n_copied_from_end, cur);
}

template <typename SpriteType>
void sprite_sort(std::vector<SpriteType>& v) {
  std::sort(v.begin(), v.end(), [](SpriteType s1, SpriteType s2) {
      return s1.oam_.x == s2.oam_.x ? s1.oam_index_ < s2.oam_index_ : s1.oam_.x < s2.oam_.x;
    });
}

std::ostream& operator<<(std::ostream& out, const OAM& oam);
