#include <memory>
#include <fstream>
#include <iostream>

#include "types.h"
#include "main.hpp"
#include "cpu6502.hpp"
#include "ppu.hpp"
#include "bus.hpp"
#include "ops.hpp"
#include "nes000.hpp"
#include "nes004.h"
#include "util.h"
#include "header.hpp"

#include <gflags/gflags.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(xx, false, "Debug");
DEFINE_bool(cloop, false, "When dumping disassembly, detect and condense loops");
DEFINE_bool(show_raster, false, "Show raster in rendered output");
DEFINE_bool(fake_sprites, false, "Show fake sprites");
DEFINE_bool(dump_stack, false, "Dump stack");

byte prg_rom_size(NESHeader* h) {
  return (((h->prg_rom_size_msb & 0xf) << 8) | h->prg_rom_size_lsb);
}

byte chr_rom_size(NESHeader* h) {
  return (((h->prg_rom_size_msb >> 4) << 8) | h->prg_rom_size_lsb);
}

void inspect_header(NESHeader* h) {
  std::cout << "PRG-ROM size: "
            << prg_rom_size(h) * 0x4000 << std::endl
            << "CHR-ROM size: " << chr_rom_size(h) * 0x2000
            << std::endl
            << "Mapper: "
            << ((h->system_flags & 0xf0) | (h->flags6 >> 4) | ((h->mapper_flags >> 4) << 8))
            << std::endl;
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage("A NES emulator");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (argc != 2) {
    std::cerr << "Expecting a ROM filename." << std::endl;
    exit(1);
  }

  auto cpu = std::make_shared<CPU6502>();
  auto ppu = std::make_shared<PPU>();
  auto mapper = std::make_shared<MMC3>();
  Bus bus(cpu, ppu);

  std::deque<word> history;
  size_t repeating = 0;
  size_t last_period = 0;

  std::ifstream f(argv[1], std::ios::in);
  if (f) {
    f.seekg(0x10, std::ios::cur);
    size_t len = f.tellg();
    f.seekg(0, std::ios::beg);

    byte header_bytes[16];
    f.read((char*) header_bytes, 16);
    NESHeader* h = reinterpret_cast<NESHeader*>(header_bytes);

    inspect_header(h);
    std::vector<char> data;
    data.reserve(len);
    std::copy(std::istreambuf_iterator<char>(f),
              std::istreambuf_iterator<char>(),
              std::back_inserter(data));

    mapper->map(data, prg_rom_size(h), chr_rom_size(h), h);
  }

  bus.attach_cart(mapper);

  cpu->reset();
  word last_pc = 0;
  while (true) {
    if (FLAGS_cloop && last_pc != cpu->pc) {
      history.emplace_back(cpu->pc);
      last_pc = cpu->pc;
      if (history.size() >= 128) {
        history.pop_front();
      }

      auto period = history_repeating(history);
      if (period) {
        repeating++;
        cpu->set_should_dump(repeating == 1);
      } else {
        if (FLAGS_dis && repeating > 0) {
          std::cout << "\t... [last " << std::dec << last_period << " ops repeated " << std::dec
                    << repeating << " times]" << std::endl << std::endl;
        }

        repeating = 0;
        cpu->set_should_dump(true);
      }
      last_period = period;
    }
    bus.tick();
  }

  return 0;
}

#pragma clang diagnostic pop