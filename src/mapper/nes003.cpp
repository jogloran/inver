
#include <iostream>

#include "mapper.hpp"
#include "nes003.hpp"
#include "header.hpp"

Mapper::Mirroring CNROM::get_mirroring() {
  return mirroring;
}

byte CNROM::read(word addr) {
  if (addr >= 0x8000 && addr <= 0xffff) {
    return rom[addr - 0x8000];
  }
  return 0;
}

void CNROM::write(word addr, byte value) {}

void
CNROM::map(const std::vector<char>& data, byte prg_banks, byte chr_banks, const NESHeader* header) {
  rom.reserve(0x4000 * prg_banks);
  chr.reserve(0x2000 * chr_banks);

  auto cur = flash((byte*) data.data(), 0x4000 * prg_banks, rom);
  flash(cur, 0x2000 * chr_banks, chr);

  mirroring = read_mirroring(header);
}

byte CNROM::chr_read(word addr) {
  return bank(chr_bank)[addr];
}

void CNROM::chr_write(word addr, byte value) {
  if (addr >= 0x8000) {
    chr_bank = value & 0b11;
  }
}

void CNROM::reset() {
  chr_bank = 0;
  std::fill(rom.begin(), rom.end(), 0x0);
  std::fill(chr.begin(), chr.end(), 0x0);
}

byte* CNROM::bank(byte bank) {
  return chr.data() + bank * 0x1000;
}
