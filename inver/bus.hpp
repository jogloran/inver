#pragma once

#include <memory>
#include <array>

#include "types.h"
#include "ppu.hpp"
#include "cart.hpp"
#include "sdl_input.hpp"

class CPU6502;

class Bus {
public:
  Bus(std::shared_ptr<CPU6502> cpu, std::shared_ptr<PPU> ppu);
  
  void attach_cart(std::shared_ptr<Cartridge> c) {
    cart = c;
    c->connect(this);
    ppu->connect(c);
  }
  
  byte read(word addr);
  void write(word addr, byte value);
  
  void tick();
  
  byte sample_input();
  
public:
  std::shared_ptr<CPU6502> cpu;
  std::shared_ptr<PPU> ppu;
  std::shared_ptr<Cartridge> cart;
  
  std::array<byte, 0x0800> ram;
  
  long ncycles;
  bool controller_polling;
  
  byte controller_state;
  SDLInput controller1;

  void dmi(byte value);
};

