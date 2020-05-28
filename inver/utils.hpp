#pragma once

#include <array>

#include "types.h"

std::array<byte, 8> unpack_bits(byte lsb, byte msb);

const char *to_6502_flag_string(byte f);

std::ostream& hex_byte(std::ostream& out);

std::ostream& hex_word(std::ostream& out);