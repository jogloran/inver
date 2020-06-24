#pragma once

#include <memory>
#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

#include "types.h"
#include "bus.hpp"
#include "mapper.hpp"
#include "ppu.hpp"
#include "header.hpp"

class Bus;

class NROM : public Mapper {
public:
  NROM() : mirroring(Mapper::Mirroring::H) {
    std::fill(rom.begin(), rom.end(), 0);
  }

  byte read(word addr) override {

    if (addr >= 0x6000 && addr <= 0x7fff) { // PRG-RAM
      return ram[(addr % 0x1000) - 0x6000];
    } else if (addr >= 0x8000 && (addr <= 0xbfff || rom.size() > 16384)) { // PRG-ROM
      return rom[addr - 0x8000];
    } else if (addr >= 0xc000 && addr <= 0xffff) {
      return rom[addr - 0xc000];
    }

//    throw std::range_error("out of range read " + std::to_string(addr));
    return 0x0;
  }

  void write(word addr, byte value) override {
    if (addr >= 0x6000 && addr <= 0x7fff) { // PRG-RAM
      ram[(addr % 0x1000) - 0x6000] = value;
    } else if (addr >= 0x8000 && addr <= 0xbfff) { // PRG-ROM

    } else if (addr >= 0xc000 && addr <= 0xffff) {

    }
  }

  Mirroring get_mirroring() override {
    return mirroring;
  }

  void
  map(const std::vector<char>& data, byte prg_banks, byte chr_banks, NESHeader* header) override {
    rom.reserve(0x4000 * prg_banks);
    chr.reserve(0x2000 * chr_banks);

    auto cur = flash((byte*) data.data(), 0x4000 * prg_banks, rom);
    flash(cur, 0x2000 * chr_banks, chr);

    mirroring = header->flags6 & 1 ? Mirroring::V : Mirroring::H;
  }

  byte chr_read(word addr) override {
    return chr[addr];
  }

  void chr_write(word addr, byte value) override {

  }

  void reset() override {}

  std::vector<byte> rom;
  std::vector<byte> chr;
  std::array<byte, 0x1000> ram;
  Mirroring mirroring;

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(rom, chr, mirroring);
  }

  friend class cereal::access;
};

CEREAL_REGISTER_TYPE(NROM)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Mapper, NROM)
