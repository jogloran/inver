#include "mbc2.h"

#include "mmu.h"

int
MBC2::bank_no() {
  return bank;
}

byte*
MBC2::get(word loc) {
  if (loc <= 0x1fff) {
    return &mmu.cart[loc];
  } else if (loc <= 0x3fff) {
    return &mmu.cart[loc];
  } else if (loc <= 0x5fff) {
    return &mmu.cart[bank * 0x4000 + (loc - 0x4000)];
  } else if (loc <= 0x7fff) {
    return &mmu.cart[bank * 0x4000 + (loc - 0x4000)];
  } else if (loc >= 0xa000 && loc <= 0xa1ff) {
    return &external_ram[loc - 0xa000];
  }
  
  return nullptr;
}

bool
MBC2::set(word loc, byte value) {
  if (loc <= 0x1fff) {
    if ((loc & 0x100) == 0) {
      if ((value & 0xf) == 0xa) {
        // enable
        external_ram_enabled = true;
      } else {
        // disable (default)
        external_ram_enabled = false;
      }
    }
    
    return true; // Do not actually modify RAM
  } else if (loc <= 0x3fff) {
    if (loc & 0x100) {
      if (value <= 0xf) {
        bank = value & 0xf;
      }
    }
    
    return true;
  } else if (loc <= 0x7fff) {
    throw std::runtime_error("Cannot write to 0x4000-7fff");
  } else if (loc >= 0xa000 && loc <= 0xa1ff) {
    external_ram[loc - 0xa000] = value & 0xf;
    return true;
  }
  
  return false;
}
