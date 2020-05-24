#include "mbc5.h"

#include "mmu.h"
#include "cpu.h"

int
MBC5::bank_no() {
  return (bank_hi << 8) + bank;
}

byte*
MBC5::get(word loc) {
  if (loc <= 0x1fff) {
    return &mmu.cart[loc];
  } else if (loc <= 0x3fff) {
    return &mmu.cart[loc];
  } else if (loc >= 0x4000 && loc <= 0x7fff) {
    int full_bank = (bank_hi << 8) + bank; // TODO: need wraparound behaviour?
//    if (full_bank > 0x20) {
//      full_bank = 0x20;
//    }
    return &mmu.cart[full_bank * 0x4000 + (loc - 0x4000)];
  } else if (loc >= 0xa000 && loc <= 0xbfff) {
    return &external_ram[ram_bank * 0x2000 + (loc - 0xa000)];
  }
  
  return nullptr;
}

bool
MBC5::set(word loc, byte value) {
  if (loc <= 0x1fff) {
    if ((value & 0xf) == 0xa) {
      // enable
      external_ram_enabled = true;
    } else {
      // disable (default)
      external_ram_enabled = false;
    }
    
    return true; // Do not actually modify RAM
  } else if (loc <= 0x2fff) {
    bank = value; // low 8 bytes of ROM bank
    return true;
  } else if (loc <= 0x3fff) {
    bank_hi = value & 0x1;
    return true;
  } else if (loc <= 0x5fff) {
    ram_bank = value & 0xf;
    return true;
  } else if (loc >= 0xa000 && loc <= 0xbfff) {
    external_ram[ram_bank * 0x2000 + (loc - 0xa000)] = value;
    return true;
  }
  
  return false;
}
