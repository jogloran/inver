#pragma once

#include <array>
#include <cstdio>
#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

#include "header.hpp"
#include "mapper.hpp"

class MMC3 : public Mapper {
public:
  MMC3() : rom_8000_fixed(false), chr_a12_inversion(false), target_bank(0),
           mirroring_mode(Mapper::Mirroring::H), irq_period(0), irq_counter(0),
           irq_enabled(false), irq_requested_(false) {
    reset_();
  }

  void map_ram(const std::vector<char>& vector, size_t len) override;

  byte* get_ram() override;

  void reset() override {
    reset_();
  }

  void dump_mapper() override {
    for (int i = 0; i < bank_for_target.size(); ++i) {
      log("R%d -> %s bank %02x\n", i, i >= 6 ? "rom" : "chr", bank_for_target[i]);
    }
  }

  void reset_() {
    std::fill(bank_for_target.begin(), bank_for_target.end(), 0x0);
    std::fill(ram.begin(), ram.end(), 0x0);

    target_bank = irq_period = irq_counter = 0;
    rom_8000_fixed = chr_a12_inversion = false;
    mirroring_mode = Mapper::Mirroring::H;
    irq_enabled = irq_requested_ = false;
  }

  void
  map(const std::vector<char>& data, byte prg_banks, byte chr_banks,
      const NESHeader* header) override {
    rom.reserve(PRG_BANK_MULTIPLIER * prg_banks);
    if (chr_banks != 0) {
      chr.reserve(CHR_BANK_MULTIPLIER * chr_banks);
      chr_pages = chr_banks * 8;
    } else {
      chr.resize(0x2000);
      chr_pages = 8;
    }

    auto cur = flash((byte*) data.data(), PRG_BANK_MULTIPLIER * prg_banks, rom);
    if (chr_banks != 0) {
      flash(cur, CHR_BANK_MULTIPLIER * chr_banks, chr);
    }

    prg_pages = prg_banks * 2;
  }

  inline byte* bank(int bank) {
    if (bank == -1) {
      return rom.data() + rom.size() - PRG_PAGE_SIZE;
    } else if (bank == -2) {
      return rom.data() + rom.size() - 2 * PRG_PAGE_SIZE;
    }
    return rom.data() + bank * PRG_PAGE_SIZE;
  }

  inline byte* chr_bank(int bank) {
    return chr.data() + bank * CHR_PAGE_SIZE;
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
    // each tile ranges from 0xx0 to 0xxf (16 bytes)
    // there are 256 tiles (zXXy)
    // there are 2 planes (Zxxy)
    // 2kb bank can store 128 tiles
    static const size_t offsets[] = {0, 0, 1, 1, 2, 3, 4, 5};
    static const word section_start[] = {0x0, 0x0, 0x0800, 0x0800, 0x1000, 0x1400, 0x1800, 0x1c00};
    static const word section_start2[] = {0x1000, 0x1000, 0x1800, 0x1800, 0x0, 0x0400, 0x0800,
                                          0x0c00};
    size_t start = chr_a12_inversion ? 4 : 0;
    size_t i = (addr >> 10) & 7;
    // i can be 0..7, corresponds to which row it is
    size_t offset = (start + i) % 8; // offset = 3, offsets[offset] = 1
    size_t bank = bank_for_target[offsets[offset]]; // bank = 1

    auto section_start_value = (chr_a12_inversion ? section_start2 : section_start)[offset];
    return chr_bank(bank)[addr - section_start_value];
  }

  void chr_write(word addr, byte value) override {
    // each tile ranges from 0xx0 to 0xxf (16 bytes)
    // there are 256 tiles (zXXy)
    // there are 2 planes (Zxxy)
    // 2kb bank can store 128 tiles
    static const size_t offsets[] = {0, 0, 1, 1, 2, 3, 4, 5};
    static const word section_start[] = {0x0, 0x0, 0x0800, 0x0800, 0x1000, 0x1400, 0x1800, 0x1c00};
    static const word section_start2[] = {0x1000, 0x1000, 0x1800, 0x1800, 0x0, 0x0400, 0x0800,
                                          0x0c00};
    size_t start = chr_a12_inversion ? 4 : 0;
    size_t i = (addr >> 10) & 7;
    // i can be 0..7, corresponds to which row it is
    size_t offset = (start + i) % 8; // offset = 3, offsets[offset] = 1
    size_t bank = bank_for_target[offsets[offset]]; // bank = 1

    auto section_start_value = (chr_a12_inversion ? section_start2 : section_start)[offset];
    chr_bank(bank)[addr - section_start_value] = value;
  }

  Mirroring get_mirroring() override;

  bool irq_requested() override;

private:
  static constexpr int PRG_BANK_MULTIPLIER = 0x4000;
  static constexpr int CHR_BANK_MULTIPLIER = 0x2000;
  static constexpr int PRG_PAGE_SIZE = 0x2000;
  static constexpr int CHR_PAGE_SIZE = 0x400;

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

  // TODO: not yet serialised
  byte prg_pages = 0;
  byte chr_pages = 0;

  void signal_scanline() override;

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(rom, chr, ram, rom_8000_fixed,
       chr_a12_inversion, target_bank, bank_for_target,
       mirroring_mode,
       irq_period, irq_counter, irq_enabled, irq_requested_);
  }

  friend class cereal::access;
};

CEREAL_REGISTER_TYPE(MMC3)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Mapper, MMC3)