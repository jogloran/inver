#pragma once

#include <array>
#include <iostream>

#include "types.h"
#include "bus.hpp"
#include "cpu6502.hpp"

using op_t = cycle_count_t(*)(CPU6502&);

extern std::array<cycle_count_t, 256> cycle_counts;
extern std::array<op_t, 256> ops;
