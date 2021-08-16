#pragma once

#include <cstdio>
#include <iostream>
#include "types.h"
#include "bus_snes.hpp"
#include "logger.hpp"

class CPU5A22: public Logger<CPU5A22> {
public:
  void tick();

  void reset();

  void brk() {
    irq<BusSNES::Interrupt::BRK>();
  }

  void cop() {
    irq<BusSNES::Interrupt::COP>();
  }

  template<BusSNES::Interrupt rupt>
  void irq() {
//    if (rupt == BusSNES::NMI) {
    log_with_tag("irq", "NMI\n");
    push(pc.b);
    push_word(pc.addr + ((rupt == BusSNES::BRK || rupt == BusSNES::COP) ? 1 : 0));
    if (rupt == BusSNES::BRK) {
      push((p.reg & ~0x30) | 0x30);
    } else {
      push(p.reg);
    }
    p.I = 1;
    pc.b = 0x0;
    pc.c = bus->read_vector<rupt>();
//    }
  }

  /**
   * Implements the MVP/MVN (move positive/negative) opcodes.
   * @param mvp MVP if true
   */
  cycle_count_t mv_(bool mvp) {
    word count = a + 1;
    cycle_count_t ncycles {};
    word operand = read_word();
    byte src_bank = operand >> 8;
    byte dst_bank = operand & 0xff;

    while (count--) {
      dual& src = x;
      dual& dst = y;

      db = dst_bank;
      byte v = read((src_bank << 16) | src);
      write((dst_bank << 16) | dst, v);

      if (mvp) --src, --dst;
      else ++src, ++dst;

      --a;
      ncycles += 7;
    }

    return ncycles;
  }

  /**
   * Implements JSL (jump to subroutine, long).
   */
  void jsl() {
    push(pc.b);
    push_word(pc.addr + 2);
    pc.addr = read_full_addr();
  }

  /**
   * Implements the RTI (return from interrupt) instruction.
   */
  void rti() {
    pop_flags();
    pc.c = pop_word();
    if (native()) {
      pc.b = pop();
    }
  }

  void stp() {}

  /**
   * Implements RTS (return from subroutine), which matches JSR.
   */
  void rts() {
    pc.c = pop_word() + 1; // Note the + 1 for the special behaviour of RTS
  }

  /**
   * Implements RTL (return long from subroutine), which matches JSL.
   */
  void rtl() {
    dword addr = pop_word();
    byte hi = pop();
    pc.addr = ((hi << 16) | addr) + 1;
  }

  /**
   * Sets the Z flag according to whether the operand is zero.
   */
  word check_z_flag(word operand, bool op16) {
    p.Z = (operand & (op16 ? 0xffff : 0xff)) == 0;
    return operand;
  }

  /**
   * Sets the N (negative) flag according to whether the operand is <0.
   */
  word check_n_flag(word operand, bool op16) {
    p.N = (operand & (op16 ? 0x8000 : 0x80)) != 0;
    return operand;
  }

  /**
   * Potentially sets both Z and N flags based on the operand.
   */
  word check_zn_flags(word operand, bool op16) {
    check_z_flag(operand, op16);
    check_n_flag(operand, op16);
    return operand;
  }

  /**
   * Performs an 8-bit or 16-bit store.
   */
  static inline void store(dual& reg, word value, bool op16) {
    if (op16) {
      reg.w = value;
    } else {
      reg.l = value;
    }
  }

  byte pop();

  word pop_word();

  /**
   * Pushes a byte onto the stack.
   */
  void push(byte data);

  /**
   * Pushes a 16-bit value onto the stack with data starting from the given address.
   */
  void push_word(word address);

  /**
   * Pops a byte from the stack into the flags register.
   */
  void pop_flags();

  cycle_count_t branch_with_offset() {
    auto offset = static_cast<sbyte>(read_byte());
    pc.addr += offset;
    return e
           ? (((pc.addr - offset) & 0xff00) == (pc.addr & 0xff00) ? 0 : 1)
           : 0;
  }

  cycle_count_t branch_with_far_offset() {
    auto offset = static_cast<sword>(read_word());
    pc.addr += offset;
    return 0;
  }

  inline void check_page_crossing(word addr1, dword addr2) {
    if ((addr1 & 0xff00) != (addr2 & 0xff00)) crossed_page = true;
  }

  void write(dword address, byte value) const;

  byte read(dword address) const;

  inline byte read_byte() {
    return read(pc.addr++);
  }

  inline word read_word() {
    dword addr = read_byte();
    auto val = addr | (read_byte() << 8);
    return val;
  }

  /**
   * Reads a 24-bit address at the current pc value.
   */
  inline dword read_full_addr() {
    byte lo = read_byte();
    byte md = read_byte();
    byte hi = read_byte();
    return (hi << 16) | (md << 8) | lo;
  }

  /**
   * Reads a 24-bit address at a given memory location.
   */
  inline dword read_full_addr(dword addr) {
    byte lo = read(addr);
    byte md = read(addr + 1);
    byte hi = read(addr + 2);
    return (hi << 16) | (md << 8) | lo;
  }

  /**
   * Reads a 16-bit value at a given memory location.
   */
  inline word read_word(dword addr) {
    return read(addr) | (read(addr + 1) << 8);
  }

  /**
   * Reads a 32-bit value at a given memory location.
   */
  inline dword read_dword(dword addr) {
    return read(addr) | (read(addr + 1) << 8) | (read(addr + 2) << 16);
  }

#define ADDR_MODE(mode, body) word deref_##mode(bool op16) { \
  if (op16) { \
    return read_word(addr_##mode()); \
  } else { \
    return read(addr_##mode()); \
  } \
} \
dword addr_##mode() body

  ADDR_MODE(abs, {
    dword lobyte_addr = (db << 16) | read_word();
    return lobyte_addr;
  })

  ADDR_MODE(zpg, {
    return dp + read_byte(); // TODO: what if the e flag is 1 and the DL register is $00
  })

  ADDR_MODE(zpg_plus_x, {
    if (e && ((dp & 0xff) == 0)) {
      return (read_byte() + x) % 256;
    } else {
      return dp + read_byte() + x;
    }
  })

  ADDR_MODE(zpg_plus_y, {
    if (e && ((dp & 0xff) == 0)) {
      return (read_byte() + y) % 256;
    } else {
      return dp + read_byte() + y;
    }
  })

  ADDR_MODE(abs_plus_x, {
    dword addr = read_word();
    check_page_crossing(addr, addr + x);
    return (db << 16) | (addr + x);
  })

  ADDR_MODE(abs_plus_y, {
    dword addr = read_word();
    check_page_crossing(addr, addr + y);
    return (db << 16) | (addr + y);
  })

  ADDR_MODE(x_indirect, {
    byte zp_offset = read_byte();
    // Pointer is always in bank 0, even if dp+zp_offset+x exceeds 0xffff
    word ptr = read((dp + zp_offset + x) & 0xffff);
    ptr |= read((dp + zp_offset + x + 1) & 0xffff) << 8;
    return (db << 16) | ptr;
  })

  ADDR_MODE(indirect, {
    return (db << 16) | read_word(dp + read_byte());
  })

  ADDR_MODE(indirect_y, {
    byte zp_offset = read_byte();
    // Pointer is always in bank 0, even if dp+zp_offset exceeds 0xffff
    word ptr = read((dp + zp_offset) & 0xffff);
    ptr |= read((dp + zp_offset + 1) & 0xffff) << 8;
    return (db << 16) | (ptr + y);
  })

  ADDR_MODE(zpg_far, {
    dword addr = dp + read_byte();
    return read_full_addr(addr);
  })

  ADDR_MODE(stk_plus_imm_indirect_y, {
    dword addr = read_word(addr_sp_plus_imm());
    return (db << 16) | (addr + y);
  })

  ADDR_MODE(sp_plus_imm, {
    byte offset = read_byte();
    return sp.w + offset;
  })

  ADDR_MODE(zpg_far_plus_y, {
    dword base = dp + read_byte();
    dword addr = read_dword(base);
    return addr + y;
  })

  ADDR_MODE(abs_plus_x_indirect, {
    dword base = (pc.b << 16) | (read_word() + x);
    return (pc.b << 16) | read_word(base);
  })

  dword addr_same_bank_abs() {
    return (pc.b << 16) | read_word();
  }

  dword addr_same_bank_indirect() {
    return (pc.b << 16) | read_word(read_word());
  }

  dword addr_same_bank_abs_plus_x_indirect() {
    return (pc.b << 16) | read_word((pc.b << 16) | (read_word() + x));
  }

  /**
   * Returns whether we are in native (65c816) mode or emulation (6502) mode.
   */
  inline bool native() const {
    return e == 0;
  }

  dual deref_imm(bool op16) {
    if (native()) {
      if (op16) {
        return {.w = read_word()};
      } else {
        return {read_byte()};
      }
    } else {
      return {read_byte()};
    }
  }

  dword addr_abs_dword() {
    return read_full_addr();
  }

  dual deref_abs_dword(bool op16) {
    dword addr = addr_abs_dword();
    if (op16) {
      return {.w = read_word(addr)};
    } else {
      return {read(addr)};
    }
  }

  dual deref_abs_dword_plus_x(bool op16) {
    dword addr = addr_abs_dword_plus_x();
    if (op16) {
      return {.w = read_word(addr)};
    } else {
      return {read(addr)};
    }
  }

  dword addr_abs_far() {
    dword addr = read_word();
    return read_full_addr(addr);
  }

  dword addr_abs_dword_plus_x() {
    return read_full_addr() + x;
  }

  dword addr_abs_indirect() {
    dword addr = read_word();
    word target = read_word(addr);
    return (pc.b << 16) | target;
  }

  cycle_count_t observe_crossed_page() { return 0; }

  void reset_crossed_page() {}

  dual a {};
  dual x {};
  dual y {};
  dual sp {};
  word dp {}; // Direct Page (aka D register)
  byte db {}; // Data Bank (aka DBR, rr)

  // Program counter (pc), a 24-bit value
  union pc_t {
    struct {
      word c: 16;
      byte b: 8; // Program Bank (aka K register)
      byte padding: 8;
    };
    dword addr;

    template<typename Ar>
    void serialize(Ar& ar) {
      ar(addr);
    }
  } pc {};

  // Flags register
  union flags_t {
    struct {
      byte C: 1;
      byte Z: 1;
      byte I: 1;
      byte D: 1; // TODO: decimal mode not implemented (affects ADC, SBC, CMP; C and N flags, but not V)
      byte x: 1; // B in 6502, index register width flag (0=16 bits, 1=8 bits)
      byte m: 1; // Unused in 6502, A,memory width flag  (0=16 bits, 1=8 bits)
      byte V: 1;
      byte N: 1;
    };
    byte reg;

    template<typename Ar>
    void serialize(Ar& ar) {
      ar(reg);
    }
  } p = {};

  bool crossed_page = false;
  bool e = true; // (false=native, true=emulation)
  long ncycles {};
  word cycles_left {};

  BusSNES* bus;

  void connect(BusSNES* snes);

  void dump_pc();

  void dump();

  std::ostream& dump_stack(std::ostream& out);

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(a, x, y, sp, pc, p, ncycles, cycles_left, crossed_page,
       e, dp, db);
  }

  static constexpr const char* TAG = "cpu";

  void dump_m7(bool flush = false);
};