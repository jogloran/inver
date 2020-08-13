#pragma once

#include <variant>

#include "sppu.hpp"
#include "types.h"

constexpr std::array<std::array<byte, 4>, 8> bpps = {{
                                                        {2, 2, 2, 2},
                                                        {4, 4, 2, 0},
                                                        {4, 4, 0, 0},
                                                        {8, 4, 0, 0},
                                                        {8, 2, 0, 0},
                                                        {4, 2, 0, 0},
                                                        {4, 0, 0, 0},
                                                        {0, 0, 0, 0},
                                                    }};

struct Layers {
  using Pixels = std::optional<std::array<byte, 256>>;
  Pixels bg1[2];
  Pixels bg2[2];
  Pixels bg3[2];
  Pixels bg4[2];
  Pixels obj[4];
};

template <byte M>
constexpr Layers mode(SPPU& ppu) {
  Layers result {};
  constexpr auto bg1 = bpps[M][0];
  constexpr auto bg2 = bpps[M][1];
  constexpr auto bg3 = bpps[M][2];
  constexpr auto bg4 = bpps[M][3];
  if constexpr (bg1 > 0) {
    result.bg1[0] = ppu.render_row(0, 0);
    result.bg1[1] = ppu.render_row(0, 1);
  }
  if constexpr (bg2 > 0) {
    result.bg2[0] = ppu.render_row(1, 0);
    result.bg2[1] = ppu.render_row(1, 1);
  }
  if constexpr (bg3 > 0) {
    result.bg3[0] = ppu.render_row(2, 0);
    result.bg3[1] = ppu.render_row(2, 1);
  }
  if constexpr (bg4 > 0) {
    result.bg4[0] = ppu.render_row(3, 0);
    result.bg4[1] = ppu.render_row(3, 1);
  }
  for (int i = 0; i < 4; ++i) {
    result.obj[i] = ppu.render_obj(i);
  }
  return result;
}
