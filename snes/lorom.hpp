#pragma once

#include <gflags/gflags.h>

#include "mapper.hpp"

DECLARE_int32(sram);

class LoROM: public Mapper {
public:
  LoROM() {
  }

  void map(std::vector<byte>&& data) override {
    rom = data;
  }

  byte read(dword address) override {
    auto bank = address >> 16;
    auto offs = address & 0xffff;

    // handles 40 <= bank <= ff, 0000-ffff
    //         00 <= bank <= 3f, 8000-ffff
    // except WRAM:
    //         80 <= bank <= bf, 0000-1fff (mirror of 00..3f)
    //         7e <= bank <= 7f, 0000-ffff
    if (address >= 0x3f'8000 && address <= 0x3f'ffff) {
      return rom[bank * 0x8000 + (offs - 0x8000)];
    } else if (address <= 0x6f'ffff) {
      if (offs <= 0x7fff) {
        return 0;
      } else {
        return rom[bank * 0x8000 + (offs - 0x8000)];
      }
    } else if (address <= 0x7d'ffff) {
      if (offs <= 0x7fff) {
        return sram1[((bank - 0x70) * 0x8000 + offs) % FLAGS_sram];// sram
      } else {
        return rom[bank * 0x8000 + (offs - 0x8000)];
      }
    } else if (address <= 0x7e'ffff) {
    } else if (address <= 0x7f'ffff) {
    } else if (address <= 0xfd'ffff) {
      // mirrors
    } else if (address <= 0xff'ffff) {
      if (offs <= 0x7fff) {
        return sram2[(bank - 0xf0) * 0x8000 + offs];// sram
      } else {
        if (bank == 0x7fe) {
          return rom[0x3f'0000 + (offs - 0x8000)];
        } else {
          return rom[0x3f'8000 + (offs - 0x8000)];
        }
      }
    }
    return 0;
  }

  void
  write(dword address, byte value) override {
    auto bank = address >> 16;
    auto offs = address & 0xffff;

    if (bank <= 0x3f && offs >= 0x8000) {
      // rom
    } else if (address <= 0x7d'ffff) {
      if (offs <= 0x7fff) {
        sram1[((bank - 0x70) * 0x8000 + offs) % FLAGS_sram] = value;
      } else {
        //      return rom[bank * 0x8000 + (offs - 0x8000)];
      }
    } else if (address <= 0x7e'ffff) {
      // WRAM (first 64k)
    } else if (address <= 0x7f'ffff) {
      // WRAM (second 64k)
    } else if (address <= 0xfd'ffff) {
      // mirrors
    } else if (address <= 0xff'ffff) {
      if (offs <= 0x7fff) {
        sram2[(bank - 0xf0) * 0x8000 + offs] = value;
      } else {

      }
    }
  }
  void reset() override {
  }
};