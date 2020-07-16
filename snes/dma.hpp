#pragma once

#include "types.h"
#include "cpu5a22.hpp"

class DMA {
public:
  byte read(byte port) {
    switch (port) {
      case 0x0:
        return dma_params.reg;

      case 0x1:
        return B_addr;

      case 0x2:
      case 0x3:
      case 0x4:
        return a1.addr >> (8 * (port - 2));

      case 0x5:
      case 0x6:
      case 0x7:
      case 0x8:
      case 0x9:
      case 0xa:
      case 0xb:
      case 0xc:
      case 0xd:
      case 0xe:
      case 0xf: ;
    }
  }

  void write(byte port, byte value) {
    switch (port) {
      case 0x0:
        dma_params.reg = value;
        break;

      case 0x1:
        B_addr = value;
        break;

      case 0x2:
      case 0x3:
      case 0x4:
      case 0x5:
      case 0x6:
      case 0x7:
      case 0x8:
      case 0x9:
      case 0xa:
      case 0xb:
      case 0xc:
      case 0xd:
      case 0xe:
      case 0xf: ;
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
  } dma_params {};

  byte B_addr {};

  CPU5A22::pc_t a1 {}; // 43x2,3,4
  CPU5A22::pc_t das {}; // 43x5,6,7

  word hdma_ptr {};
  byte hdma_line_counter {};

  byte unused {}; // 43xb,f
};