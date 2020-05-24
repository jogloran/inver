#pragma once

#include <array>
#include <stdexcept>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>

using namespace std;

#include "sdl_input.h"
#include "types.h"
#include "apu.h"
#include "timer.h"
#include "mbc_types.h"
#include "mbc_base.h"
#include "header_type.h"
#include "rang.hpp"
#include "util.h"
#include "ppu_base.h"

DECLARE_bool(no_load);
DECLARE_bool(no_save);

extern bool in_title;

class MMU {
public:
  MMU(std::string filename, PPU& ppu, APU& apu, Timer& timer):
    path(filename),
    cart(32768, 0),
    vram_bank(0),
    rom_mapped(true), ppu(ppu), apu(apu), timer(timer),
    joypad(0xf),
    input(), mbc(),
    cgb_ram_bank(1) {
    fill(mem.begin(), mem.end(), 0);
    std::copy(rom.begin(), rom.end(), mem.begin());
    
    mem[0xf000] = 0xff;
    
    fill(bgp.begin(), bgp.end(), 0xff);

    byte header_bytes[0x50];
    std::ifstream f(path);
    f.seekg(0x100);
    f.read((char*)header_bytes, 0x50);
    Header* h = reinterpret_cast<Header*>(header_bytes);
    header = *h;
      
    long rom_size = 1 << (15 + h->rom_size);
    cart.resize(rom_size);
    
    MBC cartridge_type = h->cartridge_type;
    mbc = mbc_for(cartridge_type, *this);
    
    if (!FLAGS_no_load) {
      auto load_path = replace_path_extension(path, R"(\.gbc?)", ".sav");
      mbc->load(load_path);
    }
  
    if (!FLAGS_no_save) {
      ppu.screen->add_exit_handler([this]() {
        auto sav_path = replace_path_extension(path, R"(\.gbc?)", ".sav");
        mbc->save(sav_path);
      });
    }
      
    f.seekg(0);
    f.read((char*)cart.data(), rom_size);
  }

  void set(word loc, byte value);

  byte& vram(word loc, bool use_alt_bank);
  byte& operator[](word loc);
  
  byte effective_rom_bank_for_loc(word loc);
  
  void dump_cartridge_info() {
    std::cout << rang::style::dim << rang::fg::gray << "Title\t\t" << rang::fg::black << rang::style::reset << (char*)header.title_or_manufacturer.title << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "Type\t\t" << rang::fg::black << rang::style::reset  << header.cartridge_type << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "ROM\t\t" << rang::fg::black << rang::style::reset << (1 << (15 + header.rom_size)) << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "RAM\t\t" << rang::fg::black << rang::style::reset << static_cast<int>(header.ram_size) << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "SGB\t\t" << rang::fg::black << rang::style::reset << static_cast<int>(header.sgb == 0x3) << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "NJP\t\t" << rang::fg::black << rang::style::reset  << static_cast<int>(header.destination) << rang::fg::reset << rang::style::reset << std::endl;
  }
  
  void handle_joypad();

// private:
  static constexpr word CARTRIDGE_TYPE_OFFSET = 0x0147;

  const std::string path;

  PPU& ppu;
  APU& apu;
  Timer& timer;
  
  std::array<byte, 0x100> rom;
  std::vector<byte> cart;
  
  static constexpr int RAM_BYTES = 65536;
  std::array<byte, RAM_BYTES> mem;

  bool rom_mapped;
  byte joypad;
  
  SDLInput input;
  
  Header header;
  
  char last;
  
  std::unique_ptr<MBCBase> mbc;
  
  byte vram_bank;
  
  std::array<byte, 4096*8> cgb_ram;
  byte cgb_ram_bank;
  
  std::array<byte, 64> bgp;
  byte bgp_index;
  bool bgp_auto_increment_on_write;
  
  std::array<byte, 64> obp;
  byte obp_index;
  bool obp_auto_increment_on_write;
  
private:
  std::array<byte, 0x2000> vram_bank_mem;
};



