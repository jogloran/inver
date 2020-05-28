#pragma once

#include <array>
#include <iostream>

#include "types.h"
#include "bus.hpp"
#include "cpu6502.hpp"

using op_t = cycle_count_t(*)(CPU6502&);
struct op_record { op_t f; cycle_count_t cyc; };

extern std::array<op_record, 256> ops;
