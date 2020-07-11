#pragma once

#define OP(op, mode) [](CPU5A22& cpu) {\
  byte operand = cpu.deref_##mode(); \
  cpu.a = cpu.check_zn_flags(cpu.a op operand); \
  return cpu.observe_crossed_page(); \
}

#define GEN_A(body_macro_name) [](CPU5A22& cpu) {\
  body_macro_name(cpu.a); \
  return cpu.observe_crossed_page(); \
}

#define GEN(body_macro_name, mode) [](CPU5A22& cpu) {\
  dword addr = cpu.addr_##mode(); \
  byte operand = cpu.read(addr); \
  body_macro_name(operand); \
  cpu.write(addr, operand); \
  return cpu.observe_crossed_page(); \
}

#define ADC_GEN(mode, unary_op) [](CPU5A22& cpu) {\
  byte operand = unary_op cpu.deref_##mode(); \
  byte acc = cpu.a; \
  int addend = static_cast<int>(operand) + cpu.p.C; \
  int result = cpu.a + addend; \
  cpu.p.C = result > 255; \
  cpu.a = cpu.check_zn_flags(static_cast<byte>(result)); \
  cpu.p.V = ((acc ^ cpu.a) & (operand ^ cpu.a) & 0x80) != 0; \
  return cpu.observe_crossed_page(); \
}

#define ADC(mode) ADC_GEN(mode, +)
#define SBC(mode) ADC_GEN(mode, ~)

#define ASL_BODY(operand) \
  bool msb = (operand & 0x80) != 0; \
  operand = cpu.check_zn_flags((operand << 1) & 0xfe); \
  cpu.p.C = msb;

#define LSR_BODY(operand) \
  bool lsb = operand & 0b1; \
  operand = (operand >> 1) & 0x7f; \
  cpu.p.C = lsb; \
  cpu.p.Z = (operand == 0); \
  cpu.p.N = 0;

#define ROL_BODY(operand) \
  bool msb = (operand & 0x80) != 0; \
  operand = cpu.check_zn_flags(cpu.p.C | (operand << 1)); \
  cpu.p.C = msb;

#define ROR_BODY(operand) \
  bool lsb = operand & 0b1; \
  operand = cpu.check_zn_flags((cpu.p.C << 7) | (operand >> 1)); \
  cpu.p.C = lsb;

#define ROL_A GEN_A(ROL_BODY)
#define ROL(mode) GEN(ROL_BODY, mode)

#define ROR_A GEN_A(ROR_BODY)
#define ROR(mode) GEN(ROR_BODY, mode)

#define ASL_A GEN_A(ASL_BODY)
#define ASL(mode) GEN(ASL_BODY, mode)

#define LSR(mode) GEN(LSR_BODY, mode)
#define LSR_A [](CPU5A22& cpu) {\
  bool lsb = cpu.a & 0b1; \
  cpu.a.w = (cpu.a >> 1) & 0x7f; \
  cpu.p.C = lsb; \
  cpu.p.Z = (cpu.a == 0); \
  cpu.p.N = 0; \
  return cpu.observe_crossed_page(); \
}

// Exceptionally, STA incurs the same number of cycles
// regardless of whether the access crosses a page boundary.
#define ST_NO_PAGE_CHECK(reg, mode) [](CPU5A22& cpu) {\
  dword addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.reg); \
  cpu.reset_crossed_page(); \
  return 0; \
}

#define ST(reg, mode) [](CPU5A22& cpu) {\
  dword addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.reg); \
  return cpu.observe_crossed_page(); \
}

#define STZ(mode) [](CPU5A22& cpu) { \
  dword addr = cpu.addr_##mode(); \
  cpu.write(addr, 0x0); \
  return cpu.observe_crossed_page(); \
}

#define LD(reg, mode) [](CPU5A22& cpu) {\
  auto operand = cpu.deref_##mode(); \
  cpu.reg = cpu.check_zn_flags(operand); \
  return cpu.observe_crossed_page(); \
}

#define CMP_GEN(target, mode, addt_cycles) [](CPU5A22& cpu) {\
  byte val = cpu.deref_##mode(); \
  cpu.check_zn_flags(target - val); \
  cpu.p.C = target >= val; \
  return addt_cycles; \
}

#define CMP(mode) CMP_GEN(cpu.a, mode, cpu.observe_crossed_page())
#define CP(reg, mode) CMP_GEN(cpu.reg, mode, 0)

#define INC_GEN(mode, increment) [](CPU5A22& cpu) {\
  dword addr = cpu.addr_##mode(); \
  byte result = cpu.check_zn_flags(cpu.read(addr) + increment); \
  cpu.write(addr, result); \
  return cpu.observe_crossed_page(); \
}

#define DEC(mode) INC_GEN(mode, -1)
#define INC(mode) INC_GEN(mode, +1)

#define IN(reg) [](CPU5A22& cpu) {\
  cpu.check_zn_flags(++cpu.reg); \
  return 0; \
}

#define DE(reg) [](CPU5A22& cpu) {\
  cpu.check_zn_flags(--cpu.reg); \
  return 0; \
}

#define BIT(mode) [](CPU5A22& cpu) {\
  byte operand = cpu.deref_##mode(); \
  cpu.p.N = (operand & 0x80) != 0; \
  cpu.p.V = (operand & 0x40) != 0; \
  cpu.p.Z = (operand & cpu.a) == 0; \
  return 0; \
}

#define JSR [](CPU5A22& cpu) {\
  cpu.push_word(cpu.pc.addr + 1); \
  cpu.pc.addr = cpu.read_word(); \
  return 0; \
}

// TODO:
#define JSL [](CPU5A22& cpu) {\
  return 0; \
}

// TODO:
#define JSR_abs_plus_x_indirect [](CPU5A22& cpu) {\
  return 0; \
}

#define RTI [](CPU5A22& cpu) {\
  cpu.pop_flags(); \
  cpu.pc.addr = cpu.pop_word(); \
  return 0; \
}

#define BRANCH(cond) [](CPU5A22& cpu) {\
  if (cond) { \
    return 1 + cpu.branch_with_offset(); \
  } else { \
    ++cpu.pc.addr; \
    return 0; \
  } \
}

#define NOP        [](CPU5A22& cpu) {                             return 0; }
#define XX(len, n) [](CPU5A22& cpu) { cpu.pc.addr += len;              return n; }
#define XXX        [](CPU5A22& cpu) {                             return 0; }

#define BRK        [](CPU5A22& cpu) { cpu.brk();                  return 0; }
#define JMP(mode)  [](CPU5A22& cpu) { cpu.pc.addr = cpu.addr_##mode(); return 0; }

#define PHP        [](CPU5A22& cpu) { cpu.push(cpu.p.reg | 0x30); return 0; }
#define PLP        [](CPU5A22& cpu) { cpu.pop_flags();            return 0; }

#define PHA        [](CPU5A22& cpu) { cpu.push(cpu.a);            return 0; }
#define PLA        [](CPU5A22& cpu) { cpu.a = cpu.check_zn_flags(cpu.pop()); return 0; }
#define PHX [](CPU5A22& cpu) { cpu.push(cpu.x); return 0; }
#define PLX [](CPU5A22& cpu) { cpu.x = cpu.check_zn_flags(cpu.pop()); return 0; }
#define PHY [](CPU5A22& cpu) { cpu.push(cpu.y); return 0; }
#define PLY [](CPU5A22& cpu) { cpu.y = cpu.check_zn_flags(cpu.pop()); return 0; }

#define RTS        [](CPU5A22& cpu) { cpu.rts();                  return 0; }

#define SE(reg, v) [](CPU5A22& cpu) { cpu.p.reg = v;              return 0; }
#define TXS        [](CPU5A22& cpu) { cpu.sp = cpu.x;             return 0; }

#define COP [](CPU5A22& cpu) { return 0; }
#define BRL [](CPU5A22& cpu) { return 0; }
#define PEA [](CPU5A22& cpu) { return 0; }
#define PEI [](CPU5A22& cpu) { return 0; }
#define PER [](CPU5A22& cpu) { return 0; }
#define PHB [](CPU5A22& cpu) { return 0; }
#define PHD [](CPU5A22& cpu) { return 0; }
#define PHK [](CPU5A22& cpu) { return 0; }
#define PLB [](CPU5A22& cpu) { return 0; }
#define PLD [](CPU5A22& cpu) { return 0; }
#define TCS [](CPU5A22& cpu) { return 0; }
#define TSC [](CPU5A22& cpu) { return 0; }
#define TCD [](CPU5A22& cpu) { return 0; }
#define TDC [](CPU5A22& cpu) { return 0; }
#define WDM XX(1, 2)
#define MVP [](CPU5A22& cpu) { return 0; }
#define MVN [](CPU5A22& cpu) { return 0; }
#define RTL [](CPU5A22& cpu) { return 0; }
#define WAI [](CPU5A22& cpu) { return 0; }
#define XBA [](CPU5A22& cpu) { return 0; }
#define XCE [](CPU5A22& cpu) { bool tmp = cpu.p.C; cpu.p.C = cpu.native; cpu.native = tmp; return 0; }
#define STP [](CPU5A22& cpu) { return 0; }
#define SEP [](CPU5A22& cpu) { \
  byte mask = cpu.read_byte(); \
  cpu.p.reg |= mask; \
  return 0; \
}
#define REP [](CPU5A22& cpu) { \
  byte mask = cpu.read_byte(); \
  cpu.p.reg &= ~mask; \
  return 0; \
}
#define TSB(mode) [](CPU5A22& cpu) { return 0; }
#define TRB(mode) [](CPU5A22& cpu) { return 0; }

#define BIT_NUM [](CPU5A22& cpu) { return 0; }

#define T__(src, dst) [](CPU5A22& cpu) {\
  cpu.check_zn_flags(cpu.dst = cpu.src); \
  return 0; \
}
