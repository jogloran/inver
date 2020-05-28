#pragma once

#include <array>
#include <cstdio>

#include "header.hpp"
#include "mapper.h"

class MMC3 : public Mapper {
public:
  MMC3() : rom_8000_fixed(false), chr_a12_inversion(false), target_bank(0),
           mirroring_mode(Mapper::Mirroring::H), irq_period(0), irq_counter(0),
           irq_enabled(false), irq_requested_(false) {
    std::fill(bank_for_target.begin(), bank_for_target.end(), 0x0);
    std::fill(ram.begin(), ram.end(), 0x0);
  }

  void
  map(const std::vector<char>& data, byte prg_banks, byte chr_banks, NESHeader* header) override {
    rom.reserve(0x4000 * prg_banks);
    chr.reserve(0x2000 * chr_banks);

    flash((byte*) data.data(), 0x4000 * prg_banks);
    flash_chr((byte*) data.data() + 0x4000 * prg_banks, 0x2000 * chr_banks);
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

  inline byte* chr_bank(int bank) {
    return chr.data() + bank * 0x400;
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
      return ram[addr - 0x6000];
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

  void write(word addr, byte value) override;

  byte chr_read(word addr) override {
    static const size_t offsets[] = {0, 0, 1, 1, 2, 3, 4, 5};
    size_t start = chr_a12_inversion ? 4 : 0;
    if (addr <= 0x03ff) {
      return chr_bank(bank_for_target[0])[addr];
    } else if (addr <= 0x07ff) {
      return chr_bank(bank_for_target[0])[addr];
    } else if (addr <= 0x0bff) {
      return chr_bank(bank_for_target[1])[addr - 0x0800];
    } else if (addr <= 0x0fff) {
      return chr_bank(bank_for_target[1])[addr - 0x0800];
    } else if (addr <= 0x13ff) {
      return chr_bank(bank_for_target[2])[addr - 0x1000];
    } else if (addr <= 0x17ff) {
      return chr_bank(bank_for_target[3])[addr - 0x1400];
    } else if (addr <= 0x1bff) {
      return chr_bank(bank_for_target[4])[addr - 0x1800];
    } else if (addr <= 0x1fff) {
      return chr_bank(bank_for_target[5])[addr - 0x1c00];
    }
//
//    if (addr <= 0x03ff) {
//      return chr_bank(bank_for_target[offsets[(start + 0) % 8]])[addr];
//    } else if (addr <= 0x07ff) {
//      return chr_bank(bank_for_target[offsets[(start + 0) % 8]])[addr];
//    } else if (addr <= 0x0bff) {
//      return chr_bank(bank_for_target[offsets[(start + 1) % 8]])[addr - 0x0800];
//    } else if (addr <= 0x0fff) {
//      return chr_bank(bank_for_target[offsets[(start + 1) % 8]])[addr - 0x0800];
//    } else if (addr <= 0x13ff) {
//      return chr_bank(bank_for_target[offsets[(start + 2) % 8]])[addr - 0x1000];
//    } else if (addr <= 0x17ff) {
//      return chr_bank(bank_for_target[offsets[(start + 3) % 8]])[addr - 0x1400];
//    } else if (addr <= 0x1bff) {
//      return chr_bank(bank_for_target[offsets[(start + 4) % 8]])[addr - 0x1800];
//    } else if (addr <= 0x1fff) {
//      return chr_bank(bank_for_target[offsets[(start + 5) % 8]])[addr - 0x1c00];
//    }

//    if (addr <= 0x03ff) {
//      return chr_bank(bank_for_target[offsets[(start + 0) % 8]])[addr];
//    } else if (addr <= 0x07ff) {
//      return chr_bank(bank_for_target[offsets[(start + 0) % 8]])[addr];
//    } else if (addr <= 0x0bff) {
//      return chr_bank(bank_for_target[offsets[(start + 1) % 8]])[addr - 0x0800];
//    } else if (addr <= 0x0fff) {
//      return chr_bank(bank_for_target[offsets[(start + 1) % 8]])[addr - 0x0800];
//    } else if (addr <= 0x13ff) {
//      return chr_bank(bank_for_target[offsets[(start + 2) % 8]])[addr - 0x1000];
//    } else if (addr <= 0x17ff) {
//      return chr_bank(bank_for_target[offsets[(start + 3) % 8]])[addr - 0x1400];
//    } else if (addr <= 0x1bff) {
//      return chr_bank(bank_for_target[offsets[(start + 4) % 8]])[addr - 0x1800];
//    } else if (addr <= 0x1fff) {
//      return chr_bank(bank_for_target[offsets[(start + 5) % 8]])[addr - 0x1c00];
//    }
    return 0;
  }

  void chr_write(word addr, byte value) override {}

  Mirroring get_mirroring() override;

  bool irq_requested() override;

private:
  std::vector<byte> rom;
  std::vector<byte> chr;
  std::array<byte, 0x2000> ram;

  bool rom_8000_fixed;
  bool chr_a12_inversion;
  byte target_bank;
  std::array<byte, 8> bank_for_target;

  Mirroring mirroring_mode;
  byte irq_period;
  byte irq_counter;
  bool irq_enabled;
  bool irq_requested_;

  void signal_scanline();

  void irq_handled();
};