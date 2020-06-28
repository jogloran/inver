#pragma once

#include "mapper.hpp"
#include "types.h"

class AxROM : public Mapper {
public:
  void
  map(const std::vector<char>& data, byte prg_banks, byte chr_banks,
      const NESHeader* header) override {
    rom.reserve(PRG_BANK_MULTIPLIER * prg_banks);
    if (chr_banks != 0) {
      chr.reserve(CHR_BANK_MULTIPLIER * chr_banks);
    } else {
      chr.resize(0x2000);
    }

    auto cur = flash((byte*) data.data(), PRG_BANK_MULTIPLIER * prg_banks, rom);
    if (chr_banks != 0) {
      flash(cur, CHR_BANK_MULTIPLIER * chr_banks, chr);
    }
  }

  inline byte* bank(int bank) {
    return rom.data() + bank * PRG_PAGE_SIZE;
  }

  byte read(word addr) override {
    if (addr >= 0x8000 && addr <= 0xffff) {
      return bank(bank_no)[addr - 0x8000];
    }

    return 0;
  }

  byte chr_read(word addr) override {
    return chr[addr];
  }

  void write(word addr, byte value) override {
    if (addr >= 0x8000 && addr <= 0xffff) {
      bank_no = value & 7;
      vram_page = (value >> 4) & 1;
    }
  }

  Mirroring get_mirroring() override {
    return vram_page == 0 ? Mirroring::AAAA : Mirroring::BBBB;
  }

  void chr_write(word addr, byte value) override {
    chr[addr] = value;
  }

  void reset() override {
    bank_no = 0;
    vram_page = 0;
  }

private:
  static constexpr int PRG_BANK_MULTIPLIER = 0x4000;
  static constexpr int CHR_BANK_MULTIPLIER = 0x2000;
  static constexpr int PRG_PAGE_SIZE = 0x8000;

  std::vector<byte> rom;
  std::vector<byte> chr;

  byte bank_no = 0;
  byte vram_page = 0;
};