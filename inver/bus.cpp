#include "bus.hpp"

#include "cpu6502.hpp"

std::chrono::high_resolution_clock::time_point then;

Bus::Bus(std::shared_ptr<CPU6502> cpu, std::shared_ptr<PPU> ppu) : cpu(cpu), ppu(ppu), ncycles(0),
                                                                   controller_polling(false) {
  cpu->connect(this);
  ppu->connect(this);

  std::fill(ram.begin(), ram.end(), 0);
}

void
Bus::tick() {
//  then = std::chrono::high_resolution_clock::now();
  ppu->tick();
  if (ncycles % 3 == 0) {
    cpu->tick();
  }
  if (ppu->nmi_req) {
//    ppu->log("NMI\n");
    cpu->nmi();
    ppu->nmi_req = false;
  }

  if (ncycles % (29781*3) == 0) {
//    ppu->tm.show();
    ppu->screen.blit();
  }

  ++ncycles;


//  if (ncycles % 29781 == 0) {
//    auto now = std::chrono::high_resolution_clock::now();
//    std::cout << "frame tick: " << std::chrono::duration_cast<std::chrono::duration<double>>(
//        now - then
//    ).count() * 1000 << "ms" << std::endl;
//    then = now;
//  }
}

byte
Bus::sample_input() {
  controller1.poll();
  return static_cast<byte>(controller1.state);
}

void
Bus::write(word addr, byte value) {
  if (addr <= 0x7ff) {
    ram[addr] = value;
  } else if (addr <= 0x1fff) {
    ram[addr % 0x800] = value;
  } else if (addr <= 0x3fff) {
    ppu->select(addr & 0x7, value);
  } else if (addr <= 0x4017) {
    switch (addr) {
      case 0x4014: {
        dmi(value);
        break;
      }
      case 0x4016: {
        bool controller_polling_req = (value & 0x7) == 0x1;
//        ppu->log("controller poll req: %d\n", controller_polling_req);
        if (controller_polling && !controller_polling_req) {
          controller_state = sample_input();
        } else if (!controller_polling && controller_polling_req) {
          controller_polling = true;
        }
        break;
      }
      case 0x4017:
        break;
    }
  } else if (addr >= 0x4020) {
    cart->write(addr, value);
  }
}

byte
Bus::read(word addr) {
  if (addr <= 0x7ff) {
    return ram[addr];
  } else if (addr <= 0x1fff) {
    return ram[addr % 0x800];
  } else if (addr <= 0x3fff) { // ppu
    return ppu->select(addr & 0x7);
  } else if (addr <= 0x4017) { // apu and I/O
    switch (addr) {
      case 0x4016: {
        byte lsb = controller_state & 1;
//        ppu->log("controller read bit: %d\n", lsb);
        controller_state >>= 1;
        return lsb;
      }
      case 0x4017:
        return 0x0;
    }
    return 0;
  } else if (addr <= 0x401f) { // test mode stuff
    return 0;
  } else if (addr >= 0x4020) {
    return cart->read(addr);
  }

  throw std::range_error("out of range read");
}

void Bus::dmi(byte page) {
  word addr = page << 8;
  byte* dst = (byte*) ppu->oam.data();
  for (word src = addr; src < addr + 0x100; ++src) {
    *dst++ = read(src);
  }
  cpu->cycles_left = 514;
}

void Bus::attach_cart(std::shared_ptr<Mapper> c) {
  cart = c;
  c->connect(this);
  ppu->connect(c);
}

void Bus::reset() {
  cpu->reset();
  ppu->reset();
}
