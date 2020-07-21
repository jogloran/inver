#include "bus_snes.hpp"
#include "snes_spc/spc.h"
#include "cpu5a22.hpp"

static long n = 0;
void BusSNES::tick() {
  cpu->tick();
  ppu.tick();
  ppu.tick();ppu.tick();ppu.tick();ppu.tick();ppu.tick();
  if (dma_state == DMAState::Next) {
    dma_state = DMAState::Dma;
  } else if (dma_state == DMAState::Dma) {
    cycle_count_t dma_cycles {0};
    std::for_each(dma.begin(), dma.end(), [&dma_cycles](auto& ch) {
      // TODO: DMA can be interrupted by HDMA, i.e. run() may have to observe some state and
      // finish early (to be restarted after)
      dma_cycles += ch.run();
    });
    dma_state = DMAState::Idle;
  }
  // whyyyyyyy.
  spc_read_port(spc, spc_time++, 0);
  spc_read_port(spc, spc_time++, 1);
  spc_read_port(spc, spc_time++, 2);
  spc_read_port(spc, spc_time++, 3);
}

void BusSNES::reset() {
  cpu->reset();
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
      // PPU1 (2100-213f), APU (2140-217f), WRAM (2180-2183)
      if (offs >= 0x2134 && offs <= 0x213f) {
        return ppu.read(offs);
      }
      if (offs >= 0x2140 && offs <= 0x2143) {
        return spc_read_port(spc, spc_time++, (offs - 0x2140) % 4);
      }
      if (offs == 0x2180) {
        // WRAM read/write
        return ram[wmadd.addr++];
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
        auto value = 0x2 | (in_nmi ? 0x80 : 0x0);
        in_nmi = false;
        return value;
      }
      if (offs == 0x4211) {
        // TIMEUP
        auto value = timeup;
        timeup = 0;
        return value << 7;
      }
      if (offs == 0x4212) {
        bool vblank = ppu.state == SPPU::State::VBLANK;
        bool hblank = ppu.state == SPPU::State::HBLANK;
        bool auto_joypad_read_busy = false;
        return (vblank << 7) | (hblank << 6) | auto_joypad_read_busy;
      }

      // DMA (4300-437f), PPU2, hardware registers (4200-420d; 4210-421f)
      if (offs >= 0x4300 && offs <= 0x437f) {
        return dma[(offs >> 4) & 7].read(offs & 0xf);
      }
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
        log("spc %d <- %02x\n", (offs - 0x2140) % 4, value);
        spc_write_port(spc, spc_time++, (offs - 0x2140) % 4, value);
      }
      if (offs >= 0x2100 && offs <= 0x2133) {
        ppu.write(offs, value);
      }
      if (offs == 0x2180) {
        // WRAM read/write
        ram[wmadd.addr++] = value;
      } else if (offs == 0x2181) {
        wmadd.l = value;
      } else if (offs == 0x2182) {
        wmadd.m = value;
      } else if (offs == 0x2183) {
        wmadd.h = value;
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
      if (offs == 0x4200) {
        // NMITIMEN - Interrupt Enable
        nmi.reg = value;
      }
      // HTIMEL/HTIMEH
      if (offs == 0x4207) {
        ppu.htime.l = value;
      } else if (offs == 0x4208) {
        ppu.htime.h = value;
      }
      // VTIMEL/VTIMEH
      if (offs == 0x4209) {
        ppu.vtime.l = value;
      } else if (offs == 0x420a) {
        ppu.vtime.h = value;
      }

      if (offs == 0x420b) {
        // MDMAEN
        log("MDMAEN %d\n", value);
        for (int i = 0; i < 8; ++i) {
          if (value & (1 << i)) {
            log("DMA start %d\n", i);
          }
          dma[i].on(value & (1 << i));
        }
        dma_state = DMAState::Next;
      }
      if (offs == 0x420c) {
        // HDMAEN
        log("HDMAEN %d\n", value);
      }

      if (offs >= 0x4300 && offs <= 0x437f) {
        dma[(offs >> 4) & 7].write(offs & 0xf, value);
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

void BusSNES::vblank_nmi() {
  in_nmi = true;
  if (nmi.vblank_nmi)
    cpu->irq<NMI>();
//  std::printf("show\n");
}

void BusSNES::vblank_end() {
  in_nmi = false;
  td2.show();
}

BusSNES::BusSNES() : cpu(std::make_unique<CPU5A22>()) {
  cpu->connect(this);
  ppu.connect(this);
  td2.connect(this);
  td2.show();
  byte dma_ch_no = 0;
  std::for_each(dma.begin(), dma.end(), [this, &dma_ch_no](auto& ch) {
    ch.connect(this);
    ch.set_ch(dma_ch_no++);
  });

  spc = spc_new();
  spc_init_rom(spc, spc_rom);
  spc_reset(spc);

  if (FLAGS_audio) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);
  }
}

BusSNES::~BusSNES() {
  spc_delete(spc);
}

void BusSNES::raise_timeup() {
  if (!cpu->p.I) {
    cpu->irq<IRQ>();
  }
  timeup = true; // should this be delayed, per the docs?
}
