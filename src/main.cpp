#include <memory>
#include <fstream>
#include <iostream>
#include <cstdlib>

#include "types.h"
#include "cpu6502.hpp"
#include "ppu.hpp"
#include "bus.hpp"
#include "mapper/nes000.hpp"
#include "mapper/nes001.hpp"
#include "mapper/nes003.hpp"
#include "mapper/nes004.hpp"
#include "util.h"
#include "header.hpp"

#include <gflags/gflags.h>

std::shared_ptr<Mapper> mapper_for(byte no);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(xx, false, "Debug");
DEFINE_bool(cloop, false, "When dumping disassembly, detect and condense loops");
DEFINE_bool(show_raster, false, "Show raster in rendered output");
DEFINE_bool(fake_sprites, false, "Show fake sprites");
DEFINE_bool(dump_stack, false, "Dump stack");
DEFINE_bool(audio, true, "Enable audio");
DEFINE_string(save, "", "Save path");

int main(int argc, char** argv) {
  gflags::SetUsageMessage("A NES emulator");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (argc != 2) {
    std::cerr << "Expecting a ROM filename." << std::endl;
    std::exit(1);
  }

  std::ifstream f(argv[1], std::ios::in);
  if (!f) {
    std::cerr << "Couldn't access ROM file." << std::endl;
    std::exit(2);
  }

  std::vector<char> save_data;
  std::ifstream save(FLAGS_save, std::ios::in);
  if (save) {
    std::cerr << "Loading from " << FLAGS_save << std::endl;
    std::copy(std::istreambuf_iterator<char>(save),
              std::istreambuf_iterator<char>(),
              std::back_inserter(save_data));
  }

  auto cpu = std::make_shared<CPU6502>();
  auto ppu = std::make_shared<PPU>();
  Bus bus(cpu, ppu);

  std::deque<word> history;
  size_t repeating = 0;
  size_t last_period = 0;

  std::shared_ptr<Mapper> mapper;

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

  mapper = mapper_for(mapper_no(h));
  mapper->map(data, prg_rom_size(h), chr_rom_size(h), h);
  if (save_data.size()) {
    mapper->map_ram(save_data, save_data.size());
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

std::shared_ptr<Mapper> mapper_for(byte no) {
  switch (no) {
    case 0:
      return std::make_shared<NROM>();
    case 1:
      return std::make_shared<MMC1>();
    case 3:
      return std::make_shared<CNROM>();
    case 4:
      return std::make_shared<MMC3>();
  }
  throw std::runtime_error("Unimplemented mapper");
}

#pragma clang diagnostic pop