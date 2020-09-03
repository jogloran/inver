#pragma once

#include <array>
#include <numeric>

#include "types.h"

class SPPU;

void dump_colour_math(const SPPU& sppu);

void dump_sprite(const SPPU& sppu);

void dump_pal(const SPPU& sppu);

void dump_bg(const SPPU& sppu, byte layer);

void dump_oam_bytes(const SPPU& sppu);

void dump_oam_table(const SPPU& sppu);

void dump_oam(const SPPU& sppu, bool dump_bytes);