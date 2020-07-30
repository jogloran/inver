#pragma once

#include <cstdio>
#include <iostream>
#include "types.h"
#include "bus_snes.hpp"
#include "logger.hpp"

class CPU5A22 : public Logger<CPU5A22> {
public:
  void tick();

  void reset();

  void brk() {
    irq<BusSNES::Interrupt::BRK>();
  }

  void cop() {
    irq<BusSNES::Interrupt::COP>();
  }

  template <BusSNES::Interrupt rupt>
  void irq() {
//    if (rupt == BusSNES::NMI) {
      log("NMI\n");
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

  cycle_count_t mvp() {
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
      --src;
      --dst;
      --a;
      ncycles += 7;
    }

    return ncycles;
  }

  cycle_count_t mvn() {
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
      ++src;
      ++dst;
      --a;
      ncycles += 7;
    }

    return ncycles;
  }

  void jsl() {
    push(pc.b);
    push_word(pc.addr + 2);
    pc.addr = read_full_addr();
  }

  void rti() {
    pop_flags();
    if (native()) {
      pc.c = pop_word();
      pc.b = pop();
    } else {
      pc.c = pop_word();
    }
  }

  void stp() {}

  void rts() {
    pc.c = pop_word() + 1; // Note the + 1 for the special behaviour of RTS
  }

  void rtl() {
    dword addr = pop_word();
    byte hi = pop();
    pc.addr = ((hi << 16) | addr) + 1;
  }

  word check_zn_flags(word operand, bool op16) {
    p.Z = (operand & (op16 ? 0xffff : 0xff)) == 0;
    p.N = (operand & (op16 ? 0x8000 : 0x80)) != 0;
    return operand;
  }

  inline void store(dual& reg, word value, bool op16) {
    if (op16) {
      reg.w = value;
    } else {
      reg.l = value;
    }
  }

  byte pop();

  word pop_word();

  void push(byte data);

  void push_word(word address);

  void pop_flags();

  cycle_count_t branch_with_offset() {
    sbyte offset = static_cast<sbyte>(read_byte());
    pc.addr += offset;
    return e
           ? (((pc.addr - offset) & 0xff00) == (pc.addr & 0xff00) ? 0 : 1)
           : 0;
  }

  cycle_count_t branch_with_far_offset() {
    sword offset = static_cast<sword>(read_word());
    pc.addr += offset;
    return 0;
  }

  inline void check_page_crossing(word addr1, dword addr2) {
    if ((addr1 & 0xff00) != (addr2 & 0xff00)) crossed_page = true;
  }

  void write(dword address, byte value);

  byte read(dword address);

  inline byte read_byte() {
    return read(pc.addr++);
  }

  inline word read_word() {
    dword addr = read_byte();
    auto val = addr | (read_byte() << 8);
    return val;
  }

  inline dword read_full_addr() {
    byte lo = read_byte();
    byte md = read_byte();
    byte hi = read_byte();
    return (hi << 16) | (md << 8) | lo;
  }

  inline dword read_full_addr(dword addr) {
    byte lo = read(addr);
    byte md = read(addr + 1);
    byte hi = read(addr + 2);
    return (hi << 16) | (md << 8) | lo;
  }

  inline word read_word(dword addr) {
    return read(addr) | (read(addr + 1) << 8);
  }

  inline dword read_dword(dword addr) {
    return read(addr) | (read(addr + 1) << 8) | (read(addr + 2) << 16);
  }

  word deref_abs(bool op16) {
    if (op16) {
      return read_word(addr_abs());
    } else {
      return read(addr_abs());
    }
  }

  word deref_zpg(bool op16) {
    if (op16) {
      return read_word(addr_zpg());
    } else {
      return read(addr_zpg());
    }
  }

  word deref_zpg_plus_x(bool op16) {
    if (op16) {
      return read_word(addr_zpg_plus_x());
    } else {
      return read(addr_zpg_plus_x());
    }
  }

  word deref_zpg_plus_y(bool op16) {
    if (op16) {
      return read_word(addr_zpg_plus_y());
    } else {
      return read(addr_zpg_plus_y());
    }
  }

  word deref_abs_plus_x(bool op16) {
    if (op16) {
      return read_word(addr_abs_plus_x());
    } else {
      return read(addr_abs_plus_x());
    }
  }

  word deref_abs_plus_y(bool op16) {
    if (op16) {
      return read_word(addr_abs_plus_y());
    } else {
      return read(addr_abs_plus_y());
    }
  }

  word deref_indirect(bool op16) {
    if (op16) {
      return read_word(addr_indirect());
    } else {
      return read(addr_indirect());
    }
  }

  word deref_x_indirect(bool op16) {
    if (op16) {
      return read_word(addr_x_indirect());
    } else {
      return read(addr_x_indirect());
    }
  }

  word deref_indirect_y(bool op16) {
    if (op16) {
      return read_word(addr_indirect_y());
    } else {
      return read(addr_indirect_y());
    }
  }

  word deref_zpg_far(bool op16) {
    if (op16) {
      return read_word(addr_zpg_far());
    } else {
      return read(addr_zpg_far());
    }
  }

  word deref_stk_plus_imm_indirect_y(bool op16) {
    if (op16) {
      return read_word(addr_stk_plus_imm_indirect_y());
    } else {
      return read(addr_stk_plus_imm_indirect_y());
    }
  }

  word deref_sp_plus_imm(bool op16) {
    if (op16) {
      return read_word(addr_sp_plus_imm());
    } else {
      return read(addr_sp_plus_imm());
    }
  }

  word deref_zpg_far_plus_y(bool op16) {
    if (op16) {
      return read_word(addr_zpg_far_plus_y());
    } else {
      return read(addr_zpg_far_plus_y());
    }
  }

  word deref_abs_plus_x_indirect(bool op16) {
    if (op16) {
      return read_word(addr_abs_plus_x_indirect());
    } else {
      return read(addr_abs_plus_x_indirect());
    }
  }

  dword addr_same_bank_abs() {
    return (pc.b << 16) | read_word();
  }

  dword addr_same_bank_indirect() {
    return read_full_addr((pc.b << 16) | read_word());
  }

  dword addr_same_bank_abs_plus_x_indirect() {
    return (pc.b << 16) | read_word((pc.b << 16) | (read_word() + x));
  }

  dword addr_abs() {
    dword lobyte_addr = (db << 16) | read_word();
    return lobyte_addr;
  }

  dword addr_zpg() {
    return dp + read_byte(); // TODO: what if the e flag is 1 and the DL register is $00
  }

  dword addr_zpg_plus_x() {
    if (e && ((dp & 0xff) == 0)) {
      return (read_byte() + x) % 256;
    } else {
      return dp + read_byte() + x;
    }
  }

  dword addr_zpg_plus_y() {
    if (e && ((dp & 0xff) == 0)) {
      return (read_byte() + y) % 256;
    } else {
      return dp + read_byte() + y;
    }
  }

  dword addr_abs_plus_x() {
    dword addr = read_word();
    check_page_crossing(addr, addr + x);
    return (db << 16) | (addr + x);
  }

  dword addr_abs_plus_y() {
    dword addr = read_word();
    check_page_crossing(addr, addr + y);
    return (db << 16) | (addr + y);
  }

  dword addr_x_indirect() {
    byte zp_offset = read_byte();
    word ptr = read((zp_offset + x) % 256);
    ptr |= read((zp_offset + x + 1) % 256) << 8;
    return ptr;
  }

  dword addr_indirect_y() {
    byte zp_offset = read_byte();
    word ptr = read(zp_offset);
    if (ptr & 0xff) {
      crossed_page = true;
    }
    ptr |= read((zp_offset + 1) % 256) << 8;
    return ptr + y;
  }

  dword addr_zpg_far() {
    dword addr = dp + read_byte();
    return read_full_addr(addr);
  }

  dword addr_stk_plus_imm_indirect_y() {
    dword addr = read_word(addr_sp_plus_imm());
    return (db << 16) | (addr + y);
  }

  dword addr_sp_plus_imm() {
    byte offset = read_byte();
    return sp.w + offset;
  }

  dword addr_zpg_far_plus_y() {
    dword base = dp + read_byte();
    dword addr = read_dword(base);
    return addr + y;
  }

  dword addr_abs_plus_x_indirect() {
    dword base = (pc.b << 16) | (read_word() + x);
    return (pc.b << 16) | read_word(base);
  }

  inline bool native() {
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

  dword addr_indirect() {
    return (db << 16) | read_word(dp + read_byte());
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

  union pc_t {
    struct {
      word c: 16;
      byte b: 8; // Program Bank (aka K register)
      byte padding: 8;
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
  bool e = 1; // (0=native, 1=emulation)
  long ncycles {};
  word cycles_left {};

  BusSNES* bus;

  void connect(BusSNES* snes);

  void dump_pc();

  void dump();

  std::ostream& dump_stack(std::ostream& out);

  static constexpr const char* TAG = "cpu";
};