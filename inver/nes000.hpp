#pragma once

#include <memory>
#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

#include "types.h"
#include "bus.hpp"
#include "mapper.h"
#include "ppu.hpp"
#include "header.hpp"

class Bus;

class NROM : public Mapper {
public:
  NROM(): mirroring(Mapper::Mirroring::Unknown) {
    std::fill(rom.begin(), rom.end(), 0);
  }

  void connect(Bus* b) override {
    bus = b;
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

  void map(const std::vector<char>& data, byte prg_banks, byte chr_banks, NESHeader* header) override {
    rom.reserve(0x4000 * prg_banks);
    chr.reserve(0x2000 * chr_banks);

    flash    ((byte*)data.data()                     , 0x4000 * prg_banks);
    flash_chr((byte*)data.data() + 0x4000 * prg_banks, 0x2000 * chr_banks);

    mirroring = header->flags6 & 1 ? Mirroring::V : Mirroring::H;
  }

  byte chr_read(word addr) override {
    return chr[addr];
  }

  void chr_write(word addr, byte value) override {

  }

  void flash(byte* ptr, size_t len) {
    std::copy(ptr, ptr + len, std::back_inserter(rom));
  }

  void flash_chr(byte* ptr, size_t len) {
    std::copy(ptr, ptr + len, std::back_inserter(chr));
  }

  byte ppu_read(word addr) override {
    return ppu->ppu_read(addr);
  }

  void ppu_write(word addr, byte value) override {

  }

  Bus* bus;
  PPU* ppu;
  std::vector<byte> rom;
  std::vector<byte> chr;
  Mirroring mirroring;
};
