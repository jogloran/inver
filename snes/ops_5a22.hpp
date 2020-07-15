#pragma once

#include <array>
#include <iostream>

#include "types.h"
#include "cpu5a22.hpp"

using op_t = cycle_count_t(*)(CPU5A22&);
struct op_record { op_t f; cycle_count_t cyc; };

extern std::array<op_record, 256> ops_65c816;

