#pragma once

#include "mapper.hpp"

class HiROM: public Mapper, Logger<HiROM> {
public:
  byte read(dword address) override {
    log("%06x\n", address);
    auto bank = address >> 16;
    auto offs = address & 0xffff;

    // handles 40 <= bank <= ff, 0000-ffff
    //         00 <= bank <= 3f, 8000-ffff
    // except WRAM:
    //         80 <= bank <= bf, 0000-1fff (mirror of 00..3f)
    //         7e <= bank <= 7f, 0000-ffff

    if (bank == 0x3e && offs >= 0x6000 && offs <= 0x7fff) {
      return sram2[(bank - 0x30) * 8192 + (offs - 0x6000)];
    } else if (bank <= 0x3f) {
      return rom[bank * 0x10000 + offs];
    } else if (bank <= 0x7f) {
      return rom[(bank - 0x40) * 0x10000 + offs];
    }

    return 0;
  }

  void write(dword address, byte value) override {
    auto bank = address >> 16;
    auto offs = address & 0xffff;

    if (bank == 0x3e && offs >= 0x6000 && offs <= 0x7fff) {
      sram2[(bank - 0x30) * 8192 + (offs - 0x6000)] = value;
    }
  }
  void reset() override {
  }

  constexpr static const char* TAG = "hirom";
};