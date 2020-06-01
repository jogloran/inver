#pragma once

#include <memory>
#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

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
    // PRG-ROM
    if (addr >= 0x8000 && (addr <= 0xbfff || rom.size() > 16384)) {
      return rom[addr - 0x8000];
    } else if (addr >= 0xc000 && addr <= 0xffff) {
      return rom[addr - 0xc000];
    }

//    throw std::range_error("out of range read " + std::to_string(addr));
    return 0x0;
  }

  void write(word addr, byte value) override {
    // PRG-ROM
    if (addr >= 0x8000 && addr <= 0xbfff) {

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

  Bus* bus;
  PPU* ppu;
  std::vector<byte> rom;
  std::vector<byte> chr;
  Mirroring mirroring;
};
