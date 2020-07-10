#pragma once

#include "types.h"
#include "bus.hpp"

union dual {
  operator word() {
    return w;
  }

  dual operator++() {
    ++w;
    return *this;
  }

  dual operator--() {
    --w;
    return *this;
  }

  struct {
    byte h: 8;
    byte l: 8;
  };
  word w;
};

class CPU5A22 {
public:
  void tick();

  void reset();

  void brk() {}

  void cop() {}

  void rts() {}

  dual check_zn_flags(byte operand) { return dual{0}; }

  byte pop() { return 0; }

  word pop_word() { return 0; }

  void push(byte data) {}

  void push_word(word address) {}

  byte pop_flags() { return 0; }

  cycle_count_t branch_with_offset() { return 0; }

  inline void check_page_crossing(word addr1, word addr2) {
    if ((addr1 & 0xff00) != (addr2 & 0xff00)) crossed_page = true;
  }

  inline void write(word address, byte value) { bus->write(address, value); }

  inline byte read(word address) { return bus->read(address); }

  inline byte read_byte() {
    return read(pc.addr++);
  }

  inline word read_word() {
    word addr = read_byte();
    return addr | (read_byte() << 8);
  }

#define DEFINE_ADDR_MODE(mode, body) \
  word addr_##mode() body \
  byte deref_##mode() { \
    return read(addr_##mode()); \
  }

  DEFINE_ADDR_MODE(abs, {
    return read_word();
  })

  DEFINE_ADDR_MODE(zpg, {
    return read_byte();
  })

  DEFINE_ADDR_MODE(zpg_plus_x, {
    return (read_byte() + x) % 256;
  })

  DEFINE_ADDR_MODE(zpg_plus_y, {
    return (read_byte() + y) % 256;
  })

  DEFINE_ADDR_MODE(abs_plus_x, {
    word addr = read_word();
    check_page_crossing(addr, addr + x);
    return addr + x;
  })

  DEFINE_ADDR_MODE(abs_plus_y, {
    word addr = read_word();
    check_page_crossing(addr, addr + y);
    return addr + y;
  })

  DEFINE_ADDR_MODE(x_indirect, {
    byte zp_offset = read_byte();
    word ptr = read((zp_offset + x) % 256);
    ptr |= read((zp_offset + x + 1) % 256) << 8;
    return ptr;
  })

  DEFINE_ADDR_MODE(indirect_y, {
    byte zp_offset = read_byte();
    word ptr = read(zp_offset);
    if (ptr & 0xff) {
      crossed_page = true;
    }
    ptr |= read((zp_offset + 1) % 256) << 8;
    return ptr + y;
  })

  DEFINE_ADDR_MODE(zpg_far, {
    return 0;
  });

  DEFINE_ADDR_MODE(stk_plus_imm_indirect_y, {
    return 0;
  });

  DEFINE_ADDR_MODE(sp_plus_imm, {
    return 0;
  });

  DEFINE_ADDR_MODE(zpg_far_plus_y, {
    return 0;
  });

  DEFINE_ADDR_MODE(abs_plus_x_indirect, {
    return 0;
  });

  byte deref_imm() {
    return 0;
  }

  byte deref_al() {
    return 0;
  }

  byte deref_al_x() {
    return 0;
  }

  word addr_indirect() {
    return 0;
  }

  word addr_indirect_far() {
    return 0;
  }

  word addr_abs_dword() {
    return 0;
  }

  word addr_abs_dword_plus_x() {
    return 0;
  }

  cycle_count_t observe_crossed_page() { return 0; }

  void reset_crossed_page() {}

  dual a {};
  dual x {};
  dual y {};
  dual sp {};
  word dp {}; // Direct Page (aka D register)
  byte db {}; // Data Bank (aka K register)

  union pc_t {
    struct {
      byte padding: 8;
      byte b: 8;
      word c: 16;
    };
    dword addr;
  } pc {};

  union flags_t {
    struct {
      byte C: 1;
      byte Z: 1;
      byte I: 1;
      byte D: 1;
      byte x: 1; // B in 6502, index register width flag (0=16 bits, 1=8 bits)
      byte m: 1; // Unused in 6502, A,memory width flag  (0=16 bits, 1=8 bits)
      byte V: 1;
      byte N: 1;
    };
    byte reg;
  } p = {};

  bool crossed_page = false;
  bool native = false;
  long ncycles {};
  word cycles_left {};

  Bus* bus;
};