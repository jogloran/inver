#include "bus.hpp"
#include "cpu6502.hpp"

#include <fstream>
#include <chrono>
#include <cereal/archives/binary.hpp>

using namespace std::chrono_literals;

DECLARE_bool(audio);
DECLARE_bool(tm);
DECLARE_string(save);

Bus::Bus() : ncycles(0),
             controller_polling(false),
             sound_queue(
                 std::make_shared<Sound_Queue>()),
             paused(false) {
  cpu = std::make_shared<CPU6502>();
  ppu = std::make_shared<PPU>();
  td = std::make_shared<TD>();
  tm = std::make_shared<TM>();

  cpu->connect(this);
  ppu->connect(this);
  td->connect(this);
  tm->connect(this);

  std::fill(ram.begin(), ram.end(), 0);

  if (FLAGS_audio) {
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    sound_queue->init(44100);
    apu.sample_rate(44100);
  }
}

void
Bus::tick() {
  if (paused) {
    screen->frame_rendered(1000ms / 60);
    return;
  }

  if (FLAGS_audio && ncycles % CPU_CYCLES_PER_FRAME == 0) {
    apu.end_frame();

    static blip_sample_t buf[4096];
    long count = apu.read_samples(buf, 4096);
    sound_queue->write(buf, count);
  }

//  then = std::chrono::high_resolution_clock::now();
  ppu->tick();
  ppu->tick();
  ppu->tick();

  if (ppu->nmi_req) {
    cpu->irq<NMI>();
    ppu->nmi_req = false;
  } else if (cart->irq_requested()) {
    cpu->irq<IRQ>();
  }
  cpu->tick();

  ++ncycles;
}

byte
Bus::sample_input() {
  controller1.poll();
  return static_cast<byte>(controller1.state);
}

void
Bus::write(word addr, byte value) {
  if (addr <= 0x1fff) {
    ram[addr & 0x7ff] = value;
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
        if (controller_polling && !controller_polling_req) {
          controller_state = sample_input();
        } else if (!controller_polling && controller_polling_req) {
          controller_polling = true;
        }
        break;
      }
      case 0x4017:
        break;

      default:
        apu.write_register(addr, value);
    }
  } else if (addr >= 0x4020) {
    cart->write(addr, value);
  }
}

byte
Bus::read(word addr) {
  if (addr <= 0x1fff) {
    return ram[addr & 0x7ff];
  } else if (addr <= 0x3fff) { // ppu
    return ppu->select(addr & 0x7);
  } else if (addr <= 0x4017) { // apu and I/O
    switch (addr) {
      case 0x4016: {
        byte lsb = controller_state & 1;
        controller_state >>= 1;
        return lsb;
      }
      case 0x4017:
        return 0x0;
      case 0x4015:
        return apu.read_status();
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
  std::fill(ram.begin(), ram.end(), 0);

  cpu->reset();
  ppu->reset();
  if (cart) {
    cart->reset();
  }
}

void Bus::toggle_pause() {
  paused = !paused;
  screen->set_paused(paused);
}

void Bus::request_save() {
  if (FLAGS_save != "") {
    std::cerr << "Saving to " << FLAGS_save << std::endl;
    std::ofstream f(FLAGS_save);
    if (f) {
      std::copy(ram.begin(), ram.end(), std::ostreambuf_iterator<char>(f));
    }
  }
}

[[maybe_unused]] void Bus::dump() {
  cpu->dump_pc();
  cpu->dump();
}

void Bus::pickle(std::string filename) {
  std::ofstream ofs(filename);
  {
    cereal::BinaryOutputArchive oa(ofs);
    oa(*this);
  }
}

void Bus::unpickle(std::string filename) {
  std::ifstream ifs(filename);
  cereal::BinaryInputArchive ia(ifs);
  ia(*this);

  attach_cart(cart);
  attach_screen(screen);
  ppu->connect(this);
  cpu->connect(this);
  td->connect(this);
  tm->connect(this);
}

void Bus::attach_screen(std::shared_ptr<Screen> s) {
  screen = s;
  screen->ppu = ppu;
  screen->bus = this;
  ppu->screen = s;
}
