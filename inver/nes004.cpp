//
// Created by Daniel Tse on 24/5/20.
//

#include "nes004.h"

void MMC3::map(const std::vector<char>& vector, byte prg_banks, byte chr_banks, NESHeader* header) {

}

void MMC3::connect(Bus* bus) {

}

byte MMC3::read(word addr) {
  return 0;
}

void MMC3::write(word addr, byte value) {

}

byte MMC3::chr_read(word addr) {
  return 0;
}

void MMC3::chr_write(word addr, byte value) {

}

byte MMC3::ppu_read(word addr) {
  return 0;
}

void MMC3::ppu_write(word addr, byte value) {

}

Mapper::Mirroring MMC3::get_mirroring() {
  return Mirroring::Unknown;
}
