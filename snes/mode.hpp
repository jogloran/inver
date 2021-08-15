#pragma once

#include "sppu.hpp"
#include "types.h"

using BitDepths = std::array<byte, 4>;
constexpr std::array<BitDepths, 8> bpps = {{
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
  enum {
    BACKDROP = 0xfd,
    OBJ = 0xfe,
    MATH = 0xff
  };
  using Pal = std::array<byte, 256>;
  using Win = std::array<byte, 256>;
  template <size_t Prios>
  struct LayerData {
    Pal pal[Prios];
    Win mask;
  };

  template <size_t T>
  using Layer = LayerData<T>;
  Layer<2> bg[4];
  Layer<4> obj;
  Win math;
};

template <byte M>
constexpr Layers mode(SPPU& ppu) {
  Layers result {};
  constexpr auto bg1 = bpps[M][0];
  constexpr auto bg2 = bpps[M][1];
  constexpr auto bg3 = bpps[M][2];
  constexpr auto bg4 = bpps[M][3];
  if constexpr (bg1 > 0) {
    result.bg[0].pal[0] = ppu.render_row(0, 0);
    result.bg[0].pal[1] = ppu.render_row(0, 1);
    result.bg[0].mask = ppu.compute_mask(0);
  }
  if constexpr (bg2 > 0) {
    result.bg[1].pal[0] = ppu.render_row(1, 0);
    result.bg[1].pal[1] = ppu.render_row(1, 1);
    result.bg[1].mask = ppu.compute_mask(1);
  }
  if constexpr (bg3 > 0) {
    result.bg[2].pal[0] = ppu.render_row(2, 0);
    result.bg[2].pal[1] = ppu.render_row(2, 1);
    result.bg[2].mask = ppu.compute_mask(2);
  }
  if constexpr (bg4 > 0) {
    result.bg[3].pal[0] = ppu.render_row(3, 0);
    result.bg[3].pal[1] = ppu.render_row(3, 1);
    result.bg[3].mask = ppu.compute_mask(3);
  }
  for (int i = 0; i < 4; ++i) {
    result.obj.pal[i] = ppu.render_obj(i);
    result.obj.mask = ppu.compute_mask(Layers::OBJ);
  }
  result.math = ppu.compute_mask(Layers::MATH);
  return result;
}

template<> constexpr Layers mode<7>(SPPU& ppu) {
  Layers result {};
  result.bg[0].pal[0] = ppu.render_row_mode7(0);
  result.bg[0].mask = ppu.compute_mask(0);
  for (int i = 0; i < 4; ++i) {
    result.obj.pal[i] = ppu.render_obj(i);
    result.obj.mask = ppu.compute_mask(Layers::OBJ);
  }
  result.math = ppu.compute_mask(Layers::MATH);
  return result;
}

using LayerPriorityTable = std::vector<LayerSpec>;
extern std::array<LayerPriorityTable, 8> prios_for_mode;