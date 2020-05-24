#include <memory>
#include <fstream>
#include <iostream>

#include "types.h"
#include "main.hpp"
#include "cpu6502.hpp"
#include "ppu.hpp"
#include "bus.hpp"
#include "ops.hpp"
#include "util.h"

#include <gflags/gflags.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(xx, false, "Debug");
DEFINE_bool(cloop, false, "When dumping disassembly, detect and condense loops");
DEFINE_bool(show_raster, false, "Show raster in rendered output");

struct Header {
  byte header[4];
  byte prg_rom_size_lsb;
  byte chr_rom_size_lsb;
  byte flags6;
  byte system_flags;
  byte mapper_flags;
  byte prg_rom_size_msb;
  byte prg_ram_size;
  byte padding[5];
} __attribute__((packed, aligned(1)));

void inspect_header(Header* h) {
  std::cout << "PRG-ROM size: " << (((h->prg_rom_size_msb & 0xf) << 8) | h->prg_rom_size_lsb) * 16384 << std::endl
  << "CHR-ROM size: " << ( ((h->prg_rom_size_msb >> 4) << 8) | h->prg_rom_size_lsb) * 8192 << std::endl
  << "Mapper: " << ((h->system_flags & 0xf0) | (h->flags6 >> 4) | ((h->mapper_flags >> 4) << 8)) << std::endl;
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
  auto cart = std::make_shared<Cartridge>();
  Bus bus(cpu, ppu);
  
  std::deque<word> history;
  size_t repeating = 0;
  size_t last_period = 0;
  
  std::ifstream f(argv[1], std::ios::in);
  if (f) {
    std::cerr << "reading" << std::endl;
    byte header_bytes[16];
    f.read((char*)header_bytes, 16);
    Header* h = reinterpret_cast<Header*>(header_bytes);
    
    inspect_header(h);
    
    std::cout << "header: " << (char)h->header[0] << (char)h->header[1] << (char)h->header[2] << std::endl;
    
    std::array<byte, 0x4000> cart_data;
    f.seekg(0x10);
    f.read((char*)cart_data.data(), 0x4000);
    
    std::array<byte, 0x2000> chr_data;
    f.seekg(0x4010);
    f.read((char*)chr_data.data(), 0x2000);
    
    cart->flash(cart_data.begin(), 0x4000);
    cart->flash_chr(chr_data.begin(), 0x2000);
  }
  
  bus.attach_cart(cart);
  
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
          std::cout << "\t... [last " << std::dec << last_period << " ops repeated " << std::dec << repeating << " times]" << std::endl << std::endl;
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