#include "bus_snes.hpp"
#include "cpu5a22.hpp"
#include "debug.hpp"
#include "sdl_snes_input.hpp"
#include "snes_spc/spc.h"

#include <cereal/archives/binary.hpp>
#include <gflags/gflags.h>
#include <map>

DECLARE_bool(td);
DECLARE_int32(sram);

extern std::map<dword, PCWatchSpec> dis_pcs;

static long n = 0;

void BusSNES::tick() {
  cpu->tick();
  ppu->tick(6);

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
  ppu->reset();
}

byte BusSNES::read(dword address) {
  // causes stack overflow because dump_pc does bus->read
  //  if (auto it = dis_pcs.find(address); it != dis_pcs.end() &&
  //                                       (it->second.action & PCWatchSpec::Action::R)) {
  //    cpu->dump_pc();
  //    cpu->dump();
  //  }

  auto bank = address >> 16;
  auto offs = address & 0xffff;

  if (bank >= 0x80 && bank <= 0xfd) {
    address = address & ~(1 << 23);
    bank = address >> 16;
  }

  if (bank >= 0x0 && bank <= 0x3f && offs >= 0x6000 && offs <= 0x7fff) {
    return cart->read(address);
  } else if (bank <= 0x3f && offs <= 0x7fff) {
    if (offs <= 0x1fff) {
      return ram[offs];
    } else if (offs <= 0x20ff) {
      // unused
    } else if (offs <= 0x21ff) {
      // PPU1 (2100-213f), APU (2140-217f), WRAM (2180-2183)
      if (offs >= 0x2134 && offs <= 0x213f) {
        return ppu->read(offs);
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
      if (offs == 0x4016) {
        return io1->read(offs);
      }
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
        // HVBJOY
        bool vblank = ppu->state == SPPU::State::VBLANK;
        bool hblank = ppu->state == SPPU::State::HBLANK;
        return (vblank << 7) | (hblank << 6) | auto_joypad_read_busy;
      }
      if (offs == 0x4214) {
        // RDDIVL
        return rddiv.l;
      } else if (offs == 0x4215) {
        // RDDIVH
        return rddiv.h;
      } else if (offs == 0x4216) {
        // RDMPYL
        return rdmpy.l;
      } else if (offs == 0x4217) {
        // RDMPYH
        return rdmpy.h;
      }
      if (offs == 0x4218) {
        log_with_tag("ctrl", "sample lo %02x\n", joypad_sample_lo);
        return joypad_sample_lo;
      } else if (offs == 0x4219) {
        log_with_tag("ctrl", "sample hi %02x\n", joypad_sample_hi);
        return joypad_sample_hi;
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
      // return rom[bank * 0x8000 + (offs - 0x8000)];
    }
  } else if (bank == 0x7e) {
    // WRAM (first 64k)
    return ram[offs];
  } else if (bank == 0x7f) {
    // WRAM (second 64k)
    return ram[0x10000 + offs];
  } else {
    return cart->read(address);
  }
  return 0;
}

void BusSNES::write(dword address, byte value) {
#ifndef NDEBUG
  if (auto it = dis_pcs.find(address); it != dis_pcs.end() &&
                                       (it->second.action & PCWatchSpec::Action::W)) {
    cpu->dump_pc();
    cpu->dump();
  }
#endif

  auto bank = address >> 16;
  auto offs = address & 0xffff;

  if (bank >= 0x80 && bank <= 0xfd) {
    address = address & ~(1 << 23);
    bank = address >> 16;
  }

  if (bank >= 0x0 && bank <= 0x3f && offs >= 0x6000 && offs <= 0x7fff) {
    cart->write(address, value);
  } else if (bank <= 0x3f && offs <= 0x7fff) {
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
        ppu->write(offs, value);
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
      if (offs == 0x4016) {
        io1->write(offs, value);
      }
    } else if (offs <= 0x41ff) {
      // unused
    } else if (offs <= 0x44ff) {
      if (offs == 0x4200) {
        // NMITIMEN - Interrupt Enable
        nmi.reg = value;
        if (nmi.joypad_enable) {
          log("joypad\n");
        }
      }
      if (offs == 0x4202) {
        // WRMPYA
        wrmpya = value;
      } else if (offs == 0x4203) {
        // WRMPYB
        wrmpyb = value;
        // "For some reason, the hardware does additionally set RDDIVL=WRMPYB, and RDDIVH=00h."
        rddiv.l = wrmpyb;
        rddiv.h = 0x0;
        // TODO: no delay
        rdmpy.w = wrmpya * wrmpyb;
      } else if (offs == 0x4204) {
        // WRDIVL
        wrdivx.l = value;
      } else if (offs == 0x4205) {
        // WRDIVH
        wrdivx.h = value;
      } else if (offs == 0x4206) {
        // WRDIVB
        wrdivb = value;

        if (wrdivb == 0x0) {
          rddiv.w = 0xffff;
          rdmpy = wrdivx;
        } else {
          rddiv.w = wrdivx.w / wrdivb;
          rdmpy.w = wrdivx.w % wrdivb;
        }
      }
      // HTIMEL/HTIMEH
      if (offs == 0x4207) {
        ppu->htime.l = value;
      } else if (offs == 0x4208) {
        ppu->htime.h = value;
      }
      // VTIMEL/VTIMEH
      if (offs == 0x4209) {
        ppu->vtime.l = value;
      } else if (offs == 0x420a) {
        ppu->vtime.h = value;
      }

      if (offs == 0x420b) {
        // MDMAEN
        //        log("MDMAEN %d\n", value);
        for (int i = 0; i < 8; ++i) {
          if (value & (1 << i)) {
            log("DMA start %d\n", i);
          }
          dma[i].on(value & (1 << i), false);
        }
        dma_state = DMAState::Next;
      }
      if (offs == 0x420c) {
        // HDMAEN
        if (value != 0)
          log_with_tag("hdma", "HDMAEN %d\n", value);
        for (int i = 0; i < 8; ++i) {
          if (value & (1 << i)) {
            log_with_tag("hdma", "HDMA start %d\n", i);
          }
          dma[i].on(value & (1 << i), true);
        }
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
  } else if (bank == 0x7e) {
    // WRAM (first 64k)
    ram[offs] = value;
  } else if (bank == 0x7f) {
    // WRAM (second 64k)
    ram[0x10000 + offs] = value;
  } else {
    cart->write(address, value);
  }
}

[[noreturn]] void BusSNES::run() {
  while (true) {
    tick();
  }
}

void BusSNES::vblank_nmi() {
  auto_joypad_read_start();
  in_nmi = true;
  if (nmi.vblank_nmi)
    cpu->irq<NMI>();
  //  std::printf("show\n");
}

void BusSNES::vblank_end() {
  in_nmi = false;
  if (FLAGS_td) td2.show();
}

BusSNES::BusSNES(): cpu(std::make_unique<CPU5A22>()),
                    ppu(std::make_unique<SPPU>()),
                    io1(std::make_unique<SDLSNESController>()) {
  cpu->connect(this);
  ppu->connect(this);
  td2.connect(this);
  if (FLAGS_td) td2.show();

  byte ch_no = 0;
  std::for_each(dma.begin(), dma.end(), [this, &ch_no](auto& ch) {
    ch.connect(this);
    ch.set_ch(ch_no++);
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
  timeup = true;// should this be delayed, per the docs?
}

void BusSNES::auto_joypad_read_start() {
  auto_joypad_read_busy = false;
  word input = static_cast<SDLSNESController*>(io1.get())->sample_input();
  joypad_sample_hi = input >> 8;
  joypad_sample_lo = input & 0xff;
}

void BusSNES::pickle(std::string filename) {
  std::ofstream ofs(filename);
  {
    cereal::BinaryOutputArchive oa(ofs);
    oa(*this);
  }
}

void BusSNES::unpickle(std::string filename) {
  std::ifstream ifs(filename);
  cereal::BinaryInputArchive ia(ifs);
  ia(*this);

  ppu->connect(this);
  cpu->connect(this);
  attach_screen(screen);
}

void BusSNES::attach_screen(std::shared_ptr<Screen> s) {
  screen = s;
  ppu->screen = s.get();
  screen->connect(ppu);
}

void BusSNES::set_pc(word pc) {
  cpu->pc.addr = pc;
}
