#pragma once

#include "mapper.hpp"

class UxROM : public Mapper {
public:
  UxROM() : bank_no(0), mirroring(Mirroring::Unknown) {}

  void
  map(const std::vector<char>& vector, byte prg_banks, byte chr_banks, NESHeader* header) override;

  byte read(word addr) override;

  void write(word addr, byte value) override;

  inline byte* bank(int bank) {
    if (bank == -1) {
      return rom.data() + rom.size() - 0x4000;
    }
    return rom.data() + bank * 0x4000;
  }

  Mirroring get_mirroring() override {
    return mirroring;
  }

  void reset() override {
    bank_no = 0;
  }

private:
  std::vector<byte> rom;
  std::vector<byte> chr;
  byte bank_no;
  byte total_banks;

  Mirroring mirroring;

  void chr_write(word addr, byte value) override;

  byte chr_read(word addr) override;
};