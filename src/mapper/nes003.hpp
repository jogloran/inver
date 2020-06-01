#pragma once

#include "mapper.hpp"

class CNROM : public Mapper {
public:
  CNROM(): chr_bank(0), mirroring(Mapper::Mirroring::Unknown) {}

  Mirroring get_mirroring() override;

  void
  map(const std::vector<char>& data, byte prg_banks, byte chr_banks, NESHeader* header) override;

  byte chr_read(word addr) override;

  void chr_write(word addr, byte value) override;

  void reset() override;

  std::vector<byte> rom;
  std::vector<byte> chr;
  byte chr_bank;

  Mapper::Mirroring mirroring;

  byte* bank(byte bank);

  byte read(word addr) override;

  void write(word addr, byte value) override;
};