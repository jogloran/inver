#pragma once

#include <array>
#include <vector>

#include "types.h"
#include "snes_spc/spc.h"
#include "sppu.hpp"
#include "dma.hpp"
#include "logger.hpp"
#include "td.hpp"

#include <gflags/gflags.h>
#include <SDL2/SDL.h>

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

  void map(std::vector<byte>&& data);

  std::array<byte, 0x20000> ram {};

  std::shared_ptr<CPU5A22> cpu;
  SPPU ppu;
  TD2 td2;
  SNES_SPC* spc;
  size_t spc_time {};

  std::vector<byte> rom;

  [[noreturn]] void run();

  std::array<DMA, 8> dma;
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
  } nmi;

  bool in_nmi = false;

  bool timeup = false;

  constexpr static const char* TAG = "bus";

  void vblank_nmi();

  void vblank_end();

  template <BusSNES::Interrupt rupt>
  word read_vector() {
    constexpr word table[] = {0xffea, 0xffee, 0xffe6, 0xffe4};
    return read(table[rupt]) | (read(table[rupt] + 1) << 8);
  }

  void raise_timeup();
};