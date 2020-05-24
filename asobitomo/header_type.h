#pragma once

struct Header {
  byte nops[4];
  byte logo[0x30];
  union {
    struct {
      byte manufacturer[0x4];
      byte cgb;
    };
    struct {
      byte title[0x10];
    };
  } title_or_manufacturer;
  byte licensee[2];
  byte sgb;
  MBC cartridge_type;
  byte rom_size;
  byte ram_size;
  byte destination;
  byte old_licensee;
  byte version;
  byte checksum;
  byte global_checksum[2];
} __attribute__((packed, aligned(1)));
