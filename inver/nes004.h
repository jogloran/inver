#pragma once

#include <array>

#include "mapper.h"

class MMC3 : public Mapper {
public:
  void map(const std::vector<char>& vector, byte prg_banks, byte chr_banks) override;

  void connect(Bus* bus) override;

  byte read(word addr) override;

  void write(word addr, byte value) override;

  byte chr_read(word addr) override;

  void chr_write(word addr, byte value) override;

  byte ppu_read(word addr) override;

  void ppu_write(word addr, byte value) override;

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