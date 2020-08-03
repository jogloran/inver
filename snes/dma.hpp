#pragma once

#include <cstdio>

#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>

#include "types.h"
#include "logger.hpp"

class BusSNES;

class DMA : public Logger<DMA> {
public:
  void set_ch(byte c) {
    ch = c;
    std::snprintf(tag, 16, "dma%02x", c);
  }
  void connect(BusSNES* b) {
    bus = b;
  }

  void on(bool state, bool hdma) {
    if (hdma) hdma_enabled = state;
    else dma_enabled = state;
  }

  static constexpr sbyte A_step[] = { 1, 0, -1, 0 };

  cycle_count_t run();

  byte read(byte port) {
    auto value = 0;
    switch (port) {
      case 0x0:
        value = dma_params.reg;
        break;

      case 0x1:
        value = B_addr;
        break;

      case 0x2:
      case 0x3:
      case 0x4:
        value = a1.addr >> (8 * (port - 2));
        break;

      case 0x5:
      case 0x6:
      case 0x7:
        value = das.addr >> (8 * (port - 5));
        break;

      case 0x8:
        value = hdma_ptr & 0xff;
        break;

      case 0x9:
        value = hdma_ptr >> 8;
        break;

      case 0xa:
        value = hdma_line_counter;
        break;

      case 0xb:
        value = unused;
        break;

      case 0xc:
      case 0xd:
      case 0xe:
        value = 0; // open bus
        break;

      case 0xf:
        value = unused;
        break;
    }

//    log("DMA(%02x) [%-5s] -> %02x\n", port, command_string[port], value);

    return value;
  }

  void write(byte port, byte value) {
//    log("DMA(%02x) [%-5s] <- %02x\n", port, command_string[port], value);
    switch (port) {
      case 0x0:
        dma_params.reg = value;
        break;

      case 0x1:
        B_addr = value;
        break;

      case 0x2:
        a1.lo = value;
        break;
      case 0x3:
        a1.md = value;
        break;
      case 0x4:
        a1.hi = value;
        break;

      case 0x5:
        das.lo = value;
        break;
      case 0x6:
        das.md = value;
        break;
      case 0x7:
        das.hi = value;
        break;

      case 0x8:
        hdma_ptr = (hdma_ptr & 0xff00) | value;
        break;

      case 0x9:
        hdma_ptr = (hdma_ptr & 0x00ff) | (value << 8);
        break;

      case 0xa:
        hdma_line_counter = value;
        break;

      case 0xb:
      case 0xf:
        unused = value;
        break;

      case 0xc:
      case 0xd:
      case 0xe:
        break;
    }
  }

  union dma_params_t {
    struct {
      byte tx_type : 3;
      byte dma_A_step : 2;
      byte unused : 1;
      byte hdma_indirect_table : 1;
      byte b_to_a : 1;
    };
    byte reg;

    template <typename Ar>
    void serialize(Ar& ar) { ar(reg); }
  } dma_params {};

  byte B_addr {};

  union dword_bits_t {
    struct {
      byte lo: 8;
      byte md: 8;
      byte hi: 8; // Program Bank (aka K register)
      byte padding: 8;
    };
    dword addr;

    template <typename Ar>
    void serialize(Ar& ar) { ar(addr); }
  } pc {};

  dword_bits_t a1 {}; // 43x2,3,4
  dword_bits_t das {}; // 43x5,6,7

  word hdma_ptr {};
  byte hdma_line_counter {};

  byte unused {}; // 43xb,f

  bool dma_enabled = false;
  bool hdma_enabled = false;

  BusSNES* bus;

  static constexpr const char* TAG = "dma";
  char tag[16] = {};
  byte ch = 0xff;
  bool in_transfer = false;

  void log(const char* msg, ...);

  static constexpr const char* command_string[] = {
      "param", "B@", "@L", "@M", "@H", "len L", "len H", "hbank", "h@ L", "h@ H",
      "hline", "-", "junk", "junk", "junk", "-",
  };

  cycle_count_t hdma_init();

  void hdma_tick();

  void hdma_init(bool indirect);

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(dma_params, B_addr, pc, a1, das, hdma_ptr, hdma_line_counter,
        unused, dma_enabled, hdma_enabled, ch, in_transfer);
  }
};