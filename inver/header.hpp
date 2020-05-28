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

inline byte prg_rom_size(NESHeader* h) {
  return (((h->prg_rom_size_msb & 0xf) << 8) | h->prg_rom_size_lsb);
}

inline byte chr_rom_size(NESHeader* h) {
  return (((h->prg_rom_size_msb >> 4) << 8) | h->prg_rom_size_lsb);
}

inline int mapper_no(NESHeader* h) {
  return ((h->system_flags & 0xf0) | (h->flags6 >> 4) | ((h->mapper_flags >> 4) << 8));
}

void inspect_header(NESHeader* h);
