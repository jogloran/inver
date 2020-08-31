#pragma once

#include <array>
#include <vector>

#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/archives/binary.hpp>

#include "types.h"
#include "snes_spc/spc.h"
#include "sppu.hpp"
#include "dma.hpp"
#include "logger.hpp"
#include "td.hpp"
#include "mapper.hpp"

#include "peripheral.hpp"
#include <SDL2/SDL.h>
#include <gflags/gflags.h>
#include <random>

DECLARE_bool(audio);

constexpr unsigned char spc_rom[] = {
    0xcd, 0xef, 0xbd, 0xe8, 0x00, 0xc6, 0x1d, 0xd0, 0xfc,
    0x8f, 0xaa, 0xf4, 0x8f, 0xbb, 0xf5,
    0x78, 0xcc, 0xf4,
    0xd0, 0xfb,
    0x2f, 0x19,
    0xeb, 0xf4, 0xd0, 0xfc,
    0x7e, 0xf4, 0xd0, 0x0b,
    0xe4, 0xf5, 0xcb, 0xf4,
    0xd7, 0x00, 0xfc,
    0xd0, 0xf3,
    0xab, 0x01, 0x10, 0xef, 0x7e, 0xf4, 0x10, 0xeb,
    0xba, 0xf6, 0xda, 0x00, 0xba, 0xf4, 0xc4, 0xf4, 0xdd, 0x5d, 0xd0, 0xdb, 0x1f, 0x0, 0x0, 0xc0,
    0xff
};

class CPU5A22;

class BusSNES : public Logger<BusSNES> {
public:
  BusSNES();

  ~BusSNES();

  enum Interrupt {
    NMI, IRQ, BRK, COP
  };

  void tick();

  void reset();

  byte read(dword address);

  void write(dword address, byte value);

  void connect(std::shared_ptr<Mapper> c) {
    cart = c;
  }

  std::array<byte, 0x20000> ram {};

  std::shared_ptr<CPU5A22> cpu;
  std::shared_ptr<SPPU> ppu;
  std::shared_ptr<Screen> screen;
  std::shared_ptr<Mapper> cart;

  TD2 td2;
  SNES_SPC* spc;
  size_t spc_time {};

  std::shared_ptr<Peripheral> io1;

  byte wrmpya {};
  byte wrmpyb {};
  dual rdmpy {};

  dual wrdivx {};
  byte wrdivb {};
  dual rddiv {};

  [[noreturn]] void run();

  std::array<DMA, 8> dma {};
  enum class DMAState { Idle, Next, Dma } dma_state = DMAState::Idle;

  union wmadd_t {
    struct {
      byte l : 8;
      byte m : 8;
      byte h : 1;
    };
    struct {
      dword addr : 17;
      word unused : 15;
    };
    dword reg;

    template <typename Ar>
    void serialize(Ar& ar) { ar(reg); }
  } wmadd {};

  union nmitimen_t {
    struct {
      byte joypad_enable : 1;
      byte unused : 3;
      byte hv_irq : 2;
      byte unused2: 1;
      byte vblank_nmi : 1;
    };
    byte reg;

    template <typename Ar>
    void serialize(Ar& ar) { ar(reg); }
  } nmi;

  bool in_nmi = false;

  bool timeup = false;

  constexpr static const char* TAG = "bus";

  void frame_start() {
    std::for_each(dma.begin(), dma.end(), [](DMA& ch) {
      ch.hdma_init();
    });
  }

  void hblank_start() {
    std::for_each(dma.begin(), dma.end(), [](DMA& ch) {
      ch.hdma_tick();
    });
  }

  void vblank_nmi();

  void vblank_end();

  template <BusSNES::Interrupt rupt>
  word read_vector() {
    constexpr word table[] = {0xffea, 0xffee, 0xffe6, 0xffe4};
    return read(table[rupt]) | (read(table[rupt] + 1) << 8);
  }

  void raise_timeup();

  void auto_joypad_read_start();

  byte auto_joypad_read_busy = 0;
  byte joypad_sample_hi = 0;
  byte joypad_sample_lo = 0;

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(ram, cpu, ppu, dma, dma_state,
        wmadd, nmi, in_nmi, timeup,
        auto_joypad_read_busy, joypad_sample_hi, joypad_sample_lo);
  }

  void pickle(std::string filename);

  void unpickle(std::string filename);

  void attach_screen(std::shared_ptr<Screen> s);

  void set_pc(word rst_addr);
};