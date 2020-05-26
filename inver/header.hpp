#pragma once

#include "types.h"

struct NESHeader {
  byte header[4];
  byte prg_rom_size_lsb;
  byte chr_rom_size_lsb;
  byte flags6;
  byte system_flags;
  byte mapper_flags;
  byte prg_rom_size_msb;
  byte prg_ram_size;
  byte padding[5];
} __attribute__((packed, aligned(1)));