#pragma once

#include <memory>
#include <array>

#include "types.h"
#include "ppu.hpp"
#include "mapper.hpp"
#include "sdl_input.hpp"
#include "Simple_Apu.h"
#include "Sound_Queue.h"

static const int CPU_CYCLES_PER_FRAME = 29781;

class CPU6502;

class Bus {
public:
  Bus(std::shared_ptr<CPU6502> cpu, std::shared_ptr<PPU> ppu);
  
  void attach_cart(std::shared_ptr<Mapper> c);
  
  byte read(word addr);
  void write(word addr, byte value);
  
  void tick();
  
  byte sample_input();
  
public:
  std::shared_ptr<CPU6502> cpu;
  std::shared_ptr<PPU> ppu;
  std::shared_ptr<Mapper> cart;
  
  std::array<byte, 0x0800> ram;
  
  long ncycles;
  bool controller_polling;
  
  byte controller_state;
  SDLInput controller1;

  Simple_Apu apu;
  std::shared_ptr<Sound_Queue> sound_queue;

  void dmi(byte value);

  void reset();

  void toggle_pause();

  bool paused;

  void request_save();
};

