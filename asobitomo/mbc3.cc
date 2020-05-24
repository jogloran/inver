#include "mbc3.h"

#include "mmu.h"

int
MBC3::bank_no() {
  return (bank_hi << 5) + bank;
}

byte*
MBC3::get(word loc) {
  if (loc <= 0x1fff) {
    return &mmu.cart[loc];
  } else if (loc <= 0x3fff) {
    return &mmu.cart[loc];
  } else if (loc <= 0x5fff) {
    int full_bank = (bank_hi << 5) + bank; // TODO: need wraparound behaviour?
    return &mmu.cart[full_bank * 0x4000 + (loc - 0x4000)];
  } else if (loc <= 0x7fff) {
    int full_bank = (bank_hi << 5) + bank; // TODO: need wraparound behaviour?
    return &mmu.cart[full_bank * 0x4000 + (loc - 0x4000)];
  } else if (loc >= 0xa000 && loc <= 0xbfff) {
    return &external_ram[ram_bank * 0x2000 + (loc - 0xa000)];
  }
  
  return nullptr;
}

bool
MBC3::set(word loc, byte value) {
  if (loc <= 0x1fff) {
    if ((value & 0xf) == 0xa) {
      // enable
      external_ram_enabled = true;
    } else {
      // disable (default)
      external_ram_enabled = false;
    }
    
    return true; // Do not actually modify RAM
  } else if (loc <= 0x3fff) {
    if (value == 0x00) {
      bank = 1;
    } else {
      bank = value & 0x7f;
    }
    
    return true;
  } else if (loc <= 0x5fff) {
    /* 0x4000 - 0x5fff */
    if (value <= 0x3) {
      ram_bank = value & 0x3;
    }
    return true;
  } else if (loc >= 0xa000 && loc <= 0xbfff) {
    external_ram[ram_bank * 0x2000 + (loc - 0xa000)] = value;
    return true;
  }
  
  return false;
}

void MBC3::save(std::string path) {
  std::ofstream eram(path, std::ios::binary);
  eram.write((char*)external_ram.data(), 0x2000*4);
}

void MBC3::load(std::string path) {
  std::ifstream eram(path, std::ios::binary);
  eram.read((char*)external_ram.data(), 0x2000*4);
}
