#pragma once

#include <memory>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>

#include "types.h"
#include "bus.hpp"

class CPU6502 {
public:
  CPU6502() : a(0), x(0), y(0), sp(0xfd), pc(0), p(),
              ncycles(0), cycles_left(0),
              crossed_page(false), should_dump(true) {
    p.reg = 0b00110100;
  }

  inline byte read(word address) { return bus->read(address); }

  inline byte read_direct_byte(word addr) {
    return bus->ram[addr];
  }

  inline word read_direct_word(word addr) {
    word result = read_direct_byte(addr);
    return result | (read_direct_byte(addr + 1) << 8);
  }

  inline byte read_byte() {
    return read(pc++);
  }

  inline word read_word() {
    word addr = read_byte();
    return addr | (read_byte() << 8);
  }

  inline void write(word address, byte value) { bus->write(address, value); }

  [[maybe_unused]] void set_pc(word address);

  void tick();

  void reset();

  void push_word(word address) {
    bus->ram[SP_BASE + ((sp - 1) & 0xff)] = address & 0xff;
    bus->ram[SP_BASE + (sp & 0xff)] = address >> 8;
    sp -= 2;
  }

  void push(byte data) {
    bus->ram[SP_BASE + sp] = data;
    --sp;
  }

  inline void rts() {
    pc = pop_word() + 1; // Note the + 1 for the special behaviour of RTS
  }

  // Popping with an empty stack (sp = 0x1ff) must set sp = 0x00 and read from 0x100 instead
  byte pop() {
    auto result = bus->ram[SP_BASE + ((sp + 1) & 0xff)];
    ++sp;
    return result;
  }

  word pop_word() {
    auto result = (bus->ram[SP_BASE + ((sp + 1) & 0xff)] |
                   (bus->ram[SP_BASE + ((sp + 2) & 0xff)] << 8));
    sp += 2;
    return result;
  }

  void set_should_dump(bool dump) {
    should_dump = dump;
  }

  std::ostream& dump_stack(std::ostream&);

  inline cycle_count_t branch_with_offset() {
    sbyte offset = static_cast<sbyte>(read_byte());
    pc += offset;
    return ((pc - offset) & 0xff00) == (pc & 0xff00) ? 0 : 1;
  }

  inline cycle_count_t observe_crossed_page() {
    auto result = crossed_page;
    crossed_page = false;
    return cycle_count_t(result);
  }

  inline void reset_crossed_page() {
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

    template<typename Ar>
    void serialize(Ar& ar) {
      ar(reg);
    }
  } p;

  long ncycles;
  word cycles_left;
  bool crossed_page;
  bool should_dump;

  Bus* bus;

  constexpr static word SP_BASE = 0x100;

  inline void check_page_crossing(word addr1, word addr2) {
    if ((addr1 & 0xff00) != (addr2 & 0xff00)) crossed_page = true;
  }

#define DEFINE_ADDR_MODE(mode, body) \
  word addr_##mode() body \
  byte deref_##mode() { \
    return read(addr_##mode()); \
  }

#define DEFINE_ADDR_MODE_DIRECT(mode, body) \
  word addr_##mode() body \
  byte deref_##mode() { \
    return read_direct_byte(addr_##mode()); \
  }

  DEFINE_ADDR_MODE(abs, {
    return read_word();
  })

  DEFINE_ADDR_MODE_DIRECT(zpg, {
    return read_byte();
  })

  DEFINE_ADDR_MODE_DIRECT(zpg_plus_x, {
    return (read_byte() + x) % 256;
  })

  DEFINE_ADDR_MODE_DIRECT(zpg_plus_y, {
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
    word ptr = read_direct_byte((zp_offset + x) % 256);
    ptr |= read_direct_byte((zp_offset + x + 1) % 256) << 8;
    return ptr;
  })

  DEFINE_ADDR_MODE(indirect_y, {
    byte zp_offset = read_byte();
    word ptr = read_direct_byte(zp_offset);
    if (ptr & 0xff) {
      crossed_page = true;
    }
    ptr |= read_direct_byte((zp_offset + 1) % 256) << 8;
    return ptr + y;
  })

#undef DEFINE_ADDR_MODE

  byte deref_imm() {
    return read_byte();
  }

  word addr_indirect() {
    word addr = read_word();

    // The 6502 jmp indirect bug:
    // if addr + 1 crosses a page boundary,
    // then fetch (addr + 1) - 0x100 as the high byte instead
    word hi = (addr & 0xff) == 0xff ? (addr & 0xff00) : addr + 1;
    word ptr = read(addr) | (read(hi) << 8);
    return ptr;
  }

  void connect(Bus* other) {
    bus = other;
  }

  inline byte check_zn_flags(byte operand) {
    p.Z = (operand == 0);
    p.N = (operand & 0x80) != 0;
    return operand;
  }

  inline void pop_flags() {
    p.reg = (pop() & 0b11001111) | (p.reg & 0b00110000);
  }

  void dump();

  void dump_pc();

  template<Bus::Interrupt rupt>
  void irq() {
    if (rupt == Bus::NMI || !p.I) {
      push_word(pc);
      push((p.reg & ~0x20) | 0x20);
      p.I = 1;
      pc = bus->read_vector<rupt>();
      cycles_left = 8;
    }
  }

  // The copy of P pushed by BRK should always contain __11 ____ (the unused bits set to 1)
  // and with the I flag disabled _after_ pushing
  void brk() {
    push_word(pc + 1);
    push((p.reg & ~0x30) | 0x30);
    p.I = 1;
    pc = bus->read_vector<Bus::Interrupt::IRQ>(); // irq and brk share a vector
  }

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(a, x, y, sp, pc, p, ncycles, cycles_left, crossed_page, should_dump);
  }

  word get_initial_pc();
};
