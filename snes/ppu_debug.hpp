#pragma once

#include <array>
#include <numeric>

#include "types.h"

class SPPU;

void dump_colour_math(SPPU& sppu);

void dump_sprite(SPPU& sppu);

void dump_pal(SPPU& sppu);

void dump_bg(SPPU& sppu, byte layer);

void dump_oam_bytes(SPPU& sppu);

void dump_oam_table(SPPU& sppu);

void dump_oam(SPPU& sppu, bool dump_bytes);