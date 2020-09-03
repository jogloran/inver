#include <iostream>

#include "metadata.hpp"

void inspect(const Metadata& meta) {
  std::printf("%.*s\n", 21, meta.name);
  std::printf("Type: ");
  switch (meta.mapper) {
    case Metadata::Mapper::FastROM:
      std::puts("FastROM");
      break;
    case Metadata::Mapper::HiROM:
      std::puts("HiROM");
      break;
    case Metadata::Mapper::LoROM:
      std::puts("LoROM");
      break;
  }

  std::printf("ROM: %02x\n", meta.rom);
  std::printf("ROM size:  %d\n", meta.rom_size);
  std::printf("SRAM size: %d\n", meta.sram_size);
  std::printf("Creator:   %02x\n", meta.creator);
  std::printf("Version:   %02x\n", meta.version);
  std::printf("Checksum:  %02x\n", meta.checksum);
  std::printf("~Checksum: %02x\n", meta.checksum_comp);
  std::printf("RST:       %04x\n", meta.rst);
  std::printf("NMI:       %04x\n", meta.nmi);
}

Metadata interpret(const std::vector<byte>& data) {
  Metadata meta {};

  word offset = 0x7000;
  char* name = (char*) &data[offset + 0xfc0];
  if (std::any_of(name, name + 21, [](char c) { return !isprint(c); })) {
    offset = 0xf000;
    name = (char*) &data[offset + 0xfc0];
  }

  std::copy(name, name + 21, meta.name);

  byte rom_layout = data[offset + 0xfd5];
  if ((rom_layout & 0b110000) == 3) {
    meta.mapper = Metadata::Mapper::FastROM;
  } else {
    if (rom_layout & 1) meta.mapper = Metadata::Mapper::HiROM;
    else meta.mapper = Metadata::Mapper::LoROM;
  }

  meta.rom = data[offset + 0xfd6];
  meta.rom_size = 0x400 << data[offset + 0xfd7];
  meta.sram_size = 0x400 << data[offset + 0xfd8];
  meta.creator = byte(data[offset + 0xfd9]);
  meta.version = byte(data[offset + 0xfdb]);
  meta.checksum = byte(data[offset + 0xfde]);
  meta.checksum_comp = byte(data[offset + 0xfdc]);
  meta.rst = word(data[offset + 0xffc]) | word(data[offset + 0xffd] << 8);
  meta.nmi = word(data[offset + 0xfea]) | word(data[offset + 0xfeb] << 8);

  return meta;
}
