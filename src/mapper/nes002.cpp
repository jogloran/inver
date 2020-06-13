#include "nes002.hpp"
#include "header.hpp"

byte UxROM::read(word addr) {
  if (addr >= 0x8000 && addr <= 0xbfff) {
    return bank(bank_no)[addr - 0x8000];
  } else if (addr >= 0xc000){
    return bank(-1)[addr - 0xc000];
  }
  return 0;
}

void UxROM::write(word addr, byte value) {
  if (addr >= 0x8000) {
    bank_no = value;
  }
}

byte UxROM::chr_read(word addr) {
  return chr[addr];
}

void UxROM::chr_write(word addr, byte value) {
  chr[addr] = value;
}

void
UxROM::map(const std::vector<char>& data, byte prg_banks, byte chr_banks, NESHeader* header) {
  total_banks = prg_banks;
  rom.reserve(0x4000 * prg_banks);
  chr.reserve(0x2000);

  auto cur = flash((byte*) data.data(), 0x4000 * prg_banks, rom);
  flash(cur, 0x2000, chr);

  mirroring = header->flags6 & 1 ? Mirroring::V : Mirroring::H;
}
