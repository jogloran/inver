//
// Created by Daniel Tse on 10/7/20.
//

#include "bus_snes.hpp"
#include "snes_spc/spc.h"

void BusSNES::tick() {
  cpu.tick();
}

void BusSNES::reset() {
  cpu.reset();
}

byte BusSNES::read(dword address) {
  auto bank = address >> 16;
  auto offs = address & 0xffff;

  if (bank >= 0x80 && bank <= 0xfd) {
    address = address & ~(1 << 23);
    bank = address >> 16;
  }

  if (address <= 0x3f'ffff) {
    if (offs <= 0x1fff) {
      return ram[offs];
    } else if (offs <= 0x20ff) {
      // unused
    } else if (offs <= 0x21ff) {
      // PPU1 (2100-213f), APU (2140-217f), WRAM (2180-2183),
//      if (cpu.ncycles >= 30542863) { return 0x1; }
//      if (cpu.ncycles >= 18483715 && offs == 0x2140) { return 0x0; }
//      if (cpu.ncycles >= 18090470) { return 0xcc; }
//      if (offs == 0x2140) {
//        return 0xaa;
//      } else if (offs == 0x2141) {
//        return 0xbb;
//      }
      if (offs >= 0x2134 && offs <= 0x213f) {
        return ppu.read(offs);
      }
      if (offs >= 0x2140 && offs <= 0x2143) {
        return spc_read_port(spc, spc_time++, (offs - 0x2140) % 4);
      }
    } else if (offs <= 0x2fff) {
      // unused
    } else if (offs <= 0x3fff) {
      // DSP, SuperFX, hardware registers
    } else if (offs <= 0x40ff) {
      // Old Style Joypad Registers (4016-4017)
    } else if (offs <= 0x41ff) {
      // unused
    } else if (offs <= 0x44ff) {
      if (offs == 0x4210) {
        return 0xc2;
      }
      // DMA (4300-437f), PPU2, hardware registers (4200-420d; 4210-421f)
    } else if (offs <= 0x5fff) {
      // unused
    } else if (offs <= 0x7fff) {
      // reserved
    } else {
      return rom[bank * 0x8000 + (offs - 0x8000)];
    }
  } else if (address <= 0x6f'ffff) {
    if (offs <= 0x7fff) {
      return 0;
    } else {
      return rom[bank * 0x8000 + (offs - 0x8000)];
    }
  } else if (address <= 0x7d'ffff) {
    if (offs <= 0x7fff) {
      return 0; // sram
    } else {
      return rom[bank * 0x8000 + (offs - 0x8000)];
    }
  } else if (address <= 0x7e'ffff) {
    // WRAM (first 64k)
    return ram[offs];
  } else if (address <= 0x7f'ffff) {
    // WRAM (second 64k)
    return ram[0x10000 + offs];
  } else if (address <= 0xfd'ffff) {
    // mirrors
  } else if (address <= 0xff'ffff) {
    if (offs <= 0x7fff) {
      return 0; // sram
    } else {
      if (bank == 0x7fe) {
        return rom[0x3f'0000 + (offs - 0x8000)];
      } else {
        return rom[0x3f'8000 + (offs - 0x8000)];
      }
    }
  }
  return 0;
}

void BusSNES::write(dword address, byte value) {
  auto bank = address >> 16;
  auto offs = address & 0xffff;

  if (bank >= 0x80 && bank <= 0xfd) {
    address = address & ~(1 << 23);
    bank = address >> 16;
  }

  if (address <= 0x3f'ffff) {
    if (offs <= 0x1fff) {
      ram[offs] = value;
    } else if (offs <= 0x20ff) {
      // unused
    } else if (offs <= 0x21ff) {
      // PPU1 (2100-213f), APU (2140-217f), WRAM (2180-2183),
      if (offs >= 0x2140 && offs <= 0x2147) {
        std::printf("spc %d <- %02x\n", (offs - 0x2140) % 4, value);
        spc_write_port(spc, spc_time++, (offs - 0x2140) % 4, value);
      }
      if (offs >= 0x2100 && offs <= 0x2133) {
        ppu.write(offs, value);
      }
    } else if (offs <= 0x2fff) {
      // unused
    } else if (offs <= 0x3fff) {
      // DSP, SuperFX, hardware registers
    } else if (offs <= 0x40ff) {
      // Old Style Joypad Registers (4016-4017)
    } else if (offs <= 0x41ff) {
      // unused
    } else if (offs <= 0x44ff) {
      if (offs == 0x420b) {
        // MDMAEN
        std::printf("MDMAEN %d\n", value);
      }
      if (offs == 0x420c) {
        // HDMAEN
        std::printf("HDMAEN %d\n", value);
      }

      // DMA (4300-437f), PPU2, hardware registers (4200-420d; 4210-421f)
    } else if (offs <= 0x5fff) {
      // unused
    } else if (offs <= 0x7fff) {
      // reserved
    } else {
      // rom return rom[bank * 0x8000 + (offs - 0x8000)];
    }
  } else if (address <= 0x6f'ffff) {
    if (offs <= 0x7fff) {

    } else {
      // rom return rom[bank * 0x8000 + (offs - 0x8000)];
    }
  } else if (address <= 0x7d'ffff) {
    if (offs <= 0x7fff) {
//      return 0; // sram
    } else {
//      return rom[bank * 0x8000 + (offs - 0x8000)];
    }
  } else if (address <= 0x7e'ffff) {
    // WRAM (first 64k)
    ram[offs] = value;
  } else if (address <= 0x7f'ffff) {
    // WRAM (second 64k)
    ram[0x10000 + offs] = value;
  } else if (address <= 0xfd'ffff) {
    // mirrors
  } else if (address <= 0xff'ffff) {
    if (offs <= 0x7fff) {
//      return 0; // sram
    } else {
      if (bank == 0x7fe) {
//        return rom[0x3f'0000 + (offs - 0x8000)];
      } else {
//        return rom[0x3f'8000 + (offs - 0x8000)];
      }
    }
  }
}

void BusSNES::map(std::vector<byte>&& data) {
  rom = data;
}

[[noreturn]] void BusSNES::run() {
  while (true) {
    tick();
  }
}
