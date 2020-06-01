#pragma once

#include <array>

#include "mapper.hpp"

class MMC1 : public Mapper {
public:
  MMC1() : shift(0x10), mirroring(Mirroring::AAAA),
           prg_mode(PRGMode::FinalBankAt0xC000),
           chr_mode(CHRMode::Switch8KBBank),
           ppu_0000_bank(0),
           ppu_1000_bank(0),
           prg_rom_bank(0),
           prg_ram_disabled(false) {
    std::fill(chr.begin(), chr.end(), 0x0);
    std::fill(ram.begin(), ram.end(), 0x0);
  }

  Mirroring get_mirroring() override;

  void
  map(const std::vector<char>& vector, byte prg_banks, byte chr_banks, NESHeader* header) override;

  byte read(word addr) override;

  void write(word addr, byte value) override;

  byte chr_read(word addr) override;

  void chr_write(word addr, byte value) override;

  void reset() override;

  void connect(Bus* b) override;

  inline byte* bank(int bank) {
    if (bank == -1) {
      return rom.data() + rom.size() - 0x4000;
    }
    return rom.data() + bank * 0x4000;
  }

  void map_ram(const std::vector<char>& vector, size_t len) override;

  byte* get_ram() override;

private:
  std::vector<byte> rom;
  std::vector<byte> chr;
  std::array<byte, 0x2000> ram;
  Mirroring mirroring;
  enum class PRGMode {
    Switch32KBAt0x8000,
    FirstBankAt0x8000,
    FinalBankAt0xC000,
  };
  PRGMode prg_mode;
  enum class CHRMode {
    Switch8KBBank,
    SwitchTwo4KBBanks,
  };
  CHRMode chr_mode;

  byte ppu_0000_bank;
  byte ppu_1000_bank;
  byte prg_rom_bank;
  bool prg_ram_disabled;

  byte shift;

  void mmc1_command(word mmc1_addr, byte shift);

  byte* chr_bank(byte bank);
};
