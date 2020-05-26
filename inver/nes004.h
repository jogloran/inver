#pragma once

#include <array>

#include "header.hpp"
#include "mapper.h"

class MMC3 : public Mapper {
public:
  void
  map(const std::vector<char>& vector, byte prg_banks, byte chr_banks, NESHeader* header) override;

  void connect(Bus* bus) override;

  byte read(word addr) override;

  void irq_enable(bool enable);

  void write(word addr, byte value) override {
    if (addr >= 0x8000 && addr <= 0x9fff) {
      if (addr & 1) { // target select
        auto target_bank = value & 0b111;
        bool prg_rom_bank_mode = value & (1 << 6);
        bool chr_a12_swap = value & (1 << 7);

      } else { // bank select

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

  byte chr_read(word addr) override;

  void chr_write(word addr, byte value) override;

  byte ppu_read(word addr) override;

  void ppu_write(word addr, byte value) override;

  Mirroring get_mirroring() override;

private:
  std::array<byte, 0x2000> rom1;
  std::array<byte, 0x2000> rom2;
  std::array<byte, 0x800> chr1;
  std::array<byte, 0x800> chr2;
  std::array<byte, 0x400> chr3;
  std::array<byte, 0x400> chr4;
  std::array<byte, 0x400> chr5;
  std::array<byte, 0x400> chr6;
};