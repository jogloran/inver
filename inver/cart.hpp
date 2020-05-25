#pragma once

#include <memory>
#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

#include "types.h"

class Bus;

class Cartridge {
public:
  Cartridge() {
    std::fill(rom.begin(), rom.end(), 0);
  }

  void connect(Bus* b) {
    bus = b;
  }

  byte read(word addr) {
    // PRG-ROM
    if (addr >= 0x8000 && addr <= 0xbfff) {
      return rom[addr - 0x8000];
    } else if (addr >= 0xc000 && addr <= 0xffff) {
      return rom[addr - 0xc000];
    }

//    throw std::range_error("out of range read " + std::to_string(addr));
    return 0x0;
  }

  void write(word addr, byte value) {
    // PRG-ROM
    if (addr >= 0x8000 && addr <= 0xbfff) {

    } else if (addr >= 0xc000 && addr <= 0xffff) {

    }
  }

  byte chr_read(word addr) {
    return chr[addr];
  }

  void chr_write(word addr, byte value) {

  }

  void flash(byte* ptr, size_t len) {
    std::copy(ptr, ptr + len, rom.begin());
  }

  void flash_chr(byte* ptr, size_t len) {
    std::copy(ptr, ptr + len, chr.begin());
  }

  Bus* bus;
  std::array<byte, 0x4000> rom;
  std::array<byte, 0x2000> chr;
};
