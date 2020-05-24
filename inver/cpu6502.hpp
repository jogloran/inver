#pragma once

#include <memory>

#include "types.h"

class Bus;

class CPU6502 {
public:
  CPU6502(): a(0), x(0), y(0), sp(0xfd), pc(0), p(),
    ncycles(0), cycles_left(0),
    crossed_page(false), should_dump(true) {
    p.reg = 0b00110100;
  }
  
  byte read(word address);
  void write(word address, byte value);

  [[maybe_unused]] void set_pc(word address);
  void tick();
  void reset();
  
  void push_word(word address);
  void push(byte data);
  void rts();
  void nmi();
  byte pop();
  word pop_word();
  
  word read_word();
  void set_should_dump(bool dump) {
    should_dump = dump;
  }
  
  std::ostream& dump_stack(std::ostream&);
  
  cycle_count_t branch_with_offset();
  
  cycle_count_t observe_crossed_page() {
    auto result = crossed_page;
    crossed_page = false;
    return !!result;
  }
  
  void reset_crossed_page() {
    crossed_page = false;
  }
  
public:
  byte a, x, y;
  byte sp;
  word pc;
  union flags_t {
    struct {
        byte C: 1;
        byte Z: 1;
        byte I: 1;
        byte D: 1;
        byte B: 1;
        byte U: 1;
        byte V: 1;
        byte N: 1;
    };
    byte reg;
  } p;
  long ncycles;
  word cycles_left;
  bool crossed_page;
  bool should_dump;
  
  Bus* bus;
  
  constexpr static word SP_BASE = 0x100;
  
  inline bool crosses_page(word addr1, word addr2) {
    return (addr1 & 0xff00) != (addr2 & 0xff00);
  }
  
#define DEFINE_ADDR_MODE(mode, body) \
word addr_##mode() body \
byte deref_##mode() { \
  return read(addr_##mode()); \
}
  
  DEFINE_ADDR_MODE(abs, {
    word addr = read(pc++);
    addr |= (read(pc++) << 8);
    return addr;
  })
  
  DEFINE_ADDR_MODE(zpg, {
    return read(pc++);
  })
  
  DEFINE_ADDR_MODE(zpg_plus_x, {
    return (read(pc++) + x) % 256;
  })
  
  DEFINE_ADDR_MODE(zpg_plus_y, {
    return (read(pc++) + y) % 256;
  })
  
  DEFINE_ADDR_MODE(abs_plus_x, {
    word addr = read(pc++);
    addr |= (read(pc++) << 8);
    if (crosses_page(addr, addr + x)) {
      crossed_page = true;
    }
    return addr + x;
  })
  
  DEFINE_ADDR_MODE(abs_plus_y, {
    word addr = read(pc++);
    addr |= (read(pc++) << 8);
    if (crosses_page(addr, addr + y)) {
      crossed_page = true;
    }
    return addr + y;
  })
  
  DEFINE_ADDR_MODE(x_indirect, {
    byte zp_offset = read(pc++);
    word ptr = read((zp_offset + x) % 256);
    ptr |= read((zp_offset + x + 1) % 256) << 8;
    return ptr;
  })
  
  DEFINE_ADDR_MODE(indirect_y, {
    byte zp_offset = read(pc++);
    word ptr = read(zp_offset);
    if (ptr & 0xff) {
      crossed_page = true;
    }
    ptr |= read((zp_offset + 1) % 256) << 8;
    return ptr + y;
  })
#undef DEFINE_ADDR_MODE
  byte deref_imm() {
    return read(pc++);
  }
  
  word addr_indirect() {
    word addr = read(pc++);
    addr |= (read(pc++) << 8);
    
    // The 6502 jmp indirect bug:
    // if addr + 1 crosses a page boundary,
    // then fetch (addr + 1) - 0x100 as the high byte instead
    word hi = (addr & 0xff) == 0xff ? (addr & 0xff00) : addr + 1;
    word ptr = read(addr) | (read(hi) << 8);
    return ptr;
  }
  
  byte addr_rel() {
    sbyte offset = static_cast<sbyte>(read(pc++));
    return pc + offset;
  }
  
  void connect(Bus* other) {
    bus = other;
  }
  
  inline void check_zn_flags(byte operand) {
    p.Z = (operand == 0);
    p.N = (operand & 0x80) != 0;
  }
  
  inline void pop_flags() {
    p.reg = (pop() & 0b11001111) | (p.reg & 0b00110000);
  }
};
