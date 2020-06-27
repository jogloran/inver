#pragma once

#include <memory>
#include <array>
#include <fstream>

#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>

#include "types.h"
#include "ppu.hpp"
#include "mapper.hpp"
#include "sdl_input.hpp"
#include "Simple_Apu.h"
#include "Sound_Queue.h"
#include "family_basic_keyboard.hpp"

static const int CPU_CYCLES_PER_FRAME = 29781;

class CPU6502;
class Screen;
class TD;
class TM;

class Bus {
public:
  Bus();
  
  void attach_cart(std::shared_ptr<Mapper> c);
  
  byte read(word addr);
  void write(word addr, byte value);

  [[noreturn]] void run();

  void tick();
  
public:
  void dmi(byte value);

  void reset();

  void toggle_pause();

  void request_save();

  [[maybe_unused]] void dump();

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(cpu, ppu, cart, ram);
  }

  void pickle(std::string filename);

  void unpickle(std::string filename);

  void attach_screen(std::shared_ptr<Screen> screen);

  enum Interrupt {
    NMI, RST, IRQ
  };
  template <Interrupt rupt>
  constexpr inline word read_vector() {
    constexpr word table[] = {0xfffa, 0xfffc, 0xfffe};
    return read(table[rupt]) | (read(table[rupt] + 1) << 8);
  }

  void connect1(std::shared_ptr<Peripheral> peripheral);

  void connect2(std::shared_ptr<Peripheral> peripheral);

  std::unique_ptr<CPU6502> cpu;
  std::shared_ptr<PPU> ppu;
  std::shared_ptr<Mapper> cart;
  std::shared_ptr<Screen> screen;
  std::shared_ptr<TD> td;
  std::shared_ptr<TM> tm;

  std::array<byte, 0x0800> ram;

  long ncycles;

  std::shared_ptr<Peripheral> io1;
  std::shared_ptr<Peripheral> io2;

  Simple_Apu apu;
  std::unique_ptr<Sound_Queue> sound_queue;

  bool paused;
};
