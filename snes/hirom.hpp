#pragma once

#include <gflags/gflags.h>

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

#include "mapper.hpp"

DECLARE_int32(sram);

class HiROM: public Mapper, Logger<HiROM> {
public:
  byte read(dword address) override {
    auto bank = address >> 16;
    auto offs = address & 0xffff;

    // handles 40 <= bank <= ff, 0000-ffff
    //         00 <= bank <= 3f, 8000-ffff
    // except WRAM:
    //         80 <= bank <= bf, 0000-1fff (mirror of 00..3f)
    //         7e <= bank <= 7f, 0000-ffff

    if (bank >= 0x0 && bank <= 0x3f && offs >= 0x6000 && offs <= 0x7fff) {
      bank %= 0x10;
      return sram2[(bank * 8192) % FLAGS_sram + (offs - 0x6000)];
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

    if (bank >= 0x0 && bank <= 0x3f && offs >= 0x6000 && offs <= 0x7fff) {
      bank %= 0x10;
      sram2[(bank * 8192) % FLAGS_sram + (offs - 0x6000)] = value;
    }
  }

  void reset() override {
  }

  constexpr static const char* TAG = "hirom";
};

CEREAL_REGISTER_TYPE(HiROM)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Mapper, HiROM)