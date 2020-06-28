#pragma once

#include "types.h"
#include "mapper.hpp"

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

inline bool is_valid_header(const NESHeader* h) {
  return h->header[0] == 'N' && h->header[1] == 'E' && h->header[2] == 'S' && h->header[3] == 0x1a;
}

inline byte prg_rom_size(const NESHeader* h) {
  return (((h->prg_rom_size_msb & 0xf) << 8) | h->prg_rom_size_lsb);
}

inline byte chr_rom_size(const NESHeader* h) {
  return h->chr_rom_size_lsb;
}

inline int mapper_no(const NESHeader* h) {
  return ((h->system_flags & 0xf0) | (h->flags6 >> 4) | ((h->mapper_flags >> 4) << 8));
}

inline bool is_nes20_header(const NESHeader* h) {
  return (h->system_flags >> 2) & 3;
}

inline Mapper::Mirroring read_mirroring(const NESHeader* h) {
  return (h->flags6 & 1) ? Mapper::Mirroring::V : Mapper::Mirroring::H;
}

void inspect_header(const NESHeader* h);
