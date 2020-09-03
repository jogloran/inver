#pragma once

#include <array>
#include <numeric>

#include "types.h"

class SPPU;

class PPUDebug {
public:
  static void dump_colour_math(const SPPU& sppu);

  static void dump_sprite(const SPPU& sppu);

  static void dump_pal(const SPPU& sppu);

  static void dump_bg(const SPPU& sppu, byte layer);

  static void dump_oam_bytes(const SPPU& sppu);

  static void dump_oam_table(const SPPU& sppu);

  static void dump_oam(const SPPU& sppu, bool dump_bytes);
};