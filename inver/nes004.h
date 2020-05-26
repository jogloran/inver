#pragma once

#include <array>
#include <cstdio>

#include "header.hpp"
#include "mapper.h"

class MMC3 : public Mapper {
public:
  MMC3() : rom_8000_fixed(false), chr_a12_inversion(false), target_bank(0) {
    std::fill(bank_for_target.begin(), bank_for_target.end(), 0x0);
  }

  void
  map(const std::vector<char>& data, byte prg_banks, byte chr_banks, NESHeader* header) override {
    rom.reserve(0x4000 * prg_banks);
    chr.reserve(0x2000 * chr_banks);

    flash    ((byte*)data.data()                     , 0x4000 * prg_banks);
    flash_chr((byte*)data.data() + 0x4000 * prg_banks, 0x2000 * chr_banks);
  }

  void flash(byte* ptr, size_t len) {
    std::copy(ptr, ptr + len, std::back_inserter(rom));
  }

  void flash_chr(byte* ptr, size_t len) {
    std::copy(ptr, ptr + len, std::back_inserter(chr));
  }

  inline byte* bank(int bank) {
    if (bank == -1) {
      return rom.data() + rom.size() - 0x2000;
    } else if (bank == -2) {
      return rom.data() + rom.size() - 0x4000;
    }
    return rom.data() + bank * 0x2000;
  }

  void log(const char* msg, ...) {
    va_list args;
    static char buf[1024];
    std::sprintf(buf, "%17.17s | ", "nes004");
    std::strcat(buf, msg);
    va_start(args, msg);
    std::vprintf(buf, args);
    va_end(args);
  }

  byte read(word addr) override {
    if (addr >= 0x6000 && addr <= 0x7fff) {
      // optional PRG-RAM
    } else if (addr >= 0x8000 && addr <= 0x9fff) {
      // switchable PRG-ROM (0x8000-0x9fff) OR fixed to second-last bank
      if (rom_8000_fixed) {
        return bank(-2)[addr - 0x8000];
      } else {
        return bank(bank_for_target[6])[addr - 0x8000];
      }
    } else if (addr <= 0xbfff) {
      // switchable PRG-ROM (0xa000-0xbfff)
      return bank(bank_for_target[7])[addr - 0xa000];
    } else if (addr <= 0xdfff) {
      // PRG-ROM fixed to second-last bank (0xc000-0xdfff) OR switchable
      if (rom_8000_fixed) {
        return bank(bank_for_target[6])[addr - 0xc000];
      } else {
        return bank(-2)[addr - 0xc000];
      }
    } else {
      // PRG-ROM fixed to last bank (0xe000-0xffff)
      return bank(-1)[addr - 0xe000];
    }

    return 0;
  }

  void irq_enable(bool enable);

  void write(word addr, byte value) override {
    if (addr >= 0x8000 && addr <= 0x9fff) {
      if (addr & 1) { // bank select
        if (target_bank <= 1) value &= ~1;
        if (target_bank >= 6) value &= 0x3f;
        bank_for_target[target_bank] = value;
        log("Target bank R%d = %02x\n", target_bank, value);
      } else { // target select
        auto target = value & 0b111;
        bool prg_rom_bank_mode = value & (1 << 6);
        bool chr_a12_swap = value & (1 << 7);

        target_bank = target;
        rom_8000_fixed = prg_rom_bank_mode;
        chr_a12_inversion = chr_a12_swap;

        log("Target bank R%d %02x %02x\n", target_bank, prg_rom_bank_mode, chr_a12_swap);
      }
    } else if (addr <= 0xbfff) {
      if (addr & 1) { // PRG-RAM protect

      } else { // mirroring

      }
    } else if (addr <= 0xdfff) {
      if (addr & 1) { // IRQ reload

      } else { // IRQ latch
        auto reload_value = value;
      }
    } else {
      irq_enable(addr & 1);
    }
  }

  byte chr_read(word addr) override {
    static const size_t offsets[] = { 0, 0, 1, 1, 2, 3, 4, 5 };
    size_t start = chr_a12_inversion ? 4 : 0;

    if (addr <= 0x03ff) {
      return bank(bank_for_target[(start + 0) % 8])[addr];
    } else if (addr <= 0x07ff) {
      return bank(bank_for_target[(start + 0) % 8])[addr - 0x0400];
    } else if (addr <= 0x0bff) {
      return bank(bank_for_target[(start + 1) % 8])[addr - 0x0800];
    } else if (addr <= 0x0fff) {
      return bank(bank_for_target[(start + 1) % 8])[addr - 0x0c00];
    } else if (addr <= 0x13ff) {
      return bank(bank_for_target[(start + 2) % 8])[addr - 0x1000];
    } else if (addr <= 0x17ff) {
      return bank(bank_for_target[(start + 3) % 8])[addr - 0x1400];
    } else if (addr <= 0x1bff) {
      return bank(bank_for_target[(start + 4) % 8])[addr - 0x1800];
    } else if (addr <= 0x1fff) {
      return bank(bank_for_target[(start + 5) % 8])[addr - 0x1c00];
    }
    return 0;
  }

  void chr_write(word addr, byte value) override {}

  Mirroring get_mirroring() override;

private:
  std::vector<byte> rom;
  std::vector<byte> chr;

  bool rom_8000_fixed;
  bool chr_a12_inversion;
  byte target_bank;
  std::array<byte, 8> bank_for_target;
};