#pragma once

#define OP(op, mode) [](CPU6502& cpu) {\
  byte operand = cpu.deref_##mode(); \
  cpu.a = cpu.check_zn_flags(cpu.a op operand); \
  return cpu.observe_crossed_page(); \
}

#define GEN_A(body_macro_name) [](CPU6502& cpu) {\
  body_macro_name(cpu.a); \
  return cpu.observe_crossed_page(); \
}

#define GEN(body_macro_name, mode) [](CPU6502& cpu) {\
  word addr = cpu.addr_##mode(); \
  byte operand = cpu.read(addr); \
  body_macro_name(operand); \
  cpu.write(addr, operand); \
  return cpu.observe_crossed_page(); \
}

#define BRK [](CPU6502& cpu) { \
  cpu.p.I = 1; \
  cpu.push_word(cpu.pc + 2); \
  cpu.push(cpu.p.reg | 0b00110000); \
  return 0; \
}

#define NOP [](CPU6502& cpu) { return 0; }
#define XX(len, n) [](CPU6502& cpu) { cpu.pc += len; return n; }
#define XXX [](CPU6502& cpu) { return 0; }

#define JMP(mode) [](CPU6502& cpu) {\
  cpu.pc = cpu.addr_##mode(); \
  return 0; \
}

#define PHP [](CPU6502& cpu) {\
  cpu.push(cpu.p.reg | 0b00110000); \
  return 0; \
}

#define PHA [](CPU6502& cpu) {\
  cpu.push(cpu.a); \
  return 0; \
}

#define ADC_GEN(mode, unary_op) [](CPU6502& cpu) {\
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

#define LSR_A GEN_A(LSR_BODY)
#define LSR(mode) GEN(LSR_BODY, mode)

// Exceptionally, STA incurs the same number of cycles
// regardless of whether the access crosses a page boundary.
#define ST_NO_PAGE_CHECK(reg, mode) [](CPU6502& cpu) {\
  word addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.reg); \
  cpu.reset_crossed_page(); \
  return 0; \
}

#define ST(reg, mode) [](CPU6502& cpu) {\
  word addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.reg); \
  return cpu.observe_crossed_page(); \
}

#define LD(reg, mode) [](CPU6502& cpu) {\
  auto operand = cpu.deref_##mode(); \
  cpu.reg = cpu.check_zn_flags(operand); \
  return cpu.observe_crossed_page(); \
}

#define CMP_GEN(target, mode, addt_cycles) [](CPU6502& cpu) {\
  byte val = cpu.deref_##mode(); \
  cpu.check_zn_flags(target - val); \
  cpu.p.C = target >= val; \
  return addt_cycles; \
}

#define CMP(mode) CMP_GEN(cpu.a, mode, cpu.observe_crossed_page())
#define CP(reg, mode) CMP_GEN(cpu.reg, mode, 0)

#define INC_GEN(mode, increment) [](CPU6502& cpu) {\
  word addr = cpu.addr_##mode(); \
  byte result = cpu.check_zn_flags(cpu.read(addr) + increment); \
  cpu.write(addr, result); \
  return cpu.observe_crossed_page(); \
}

#define DEC(mode) INC_GEN(mode, -1)
#define INC(mode) INC_GEN(mode, +1)

#define JSR [](CPU6502& cpu) {\
  cpu.push_word(cpu.pc + 1); \
  cpu.pc = cpu.read_word(); \
  return 0; \
}

#define RTI [](CPU6502& cpu) {\
  cpu.pop_flags(); \
  cpu.pc = cpu.pop_word(); \
  return 0; \
}

#define RTS [](CPU6502& cpu) {\
  cpu.rts(); \
  return 0; \
}

#define BRANCH(cond) [](CPU6502& cpu) {\
  if (cond) { \
    return 1 + cpu.branch_with_offset(); \
  } else { \
    ++cpu.pc; \
    return 0; \
  } \
}

#define IN(reg) [](CPU6502& cpu) {\
  cpu.check_zn_flags(++cpu.reg); \
  return 0; \
}

#define DE(reg) [](CPU6502& cpu) {\
  cpu.check_zn_flags(--cpu.reg); \
  return 0; \
}

#define BIT(mode) [](CPU6502& cpu) {\
  byte operand = cpu.deref_##mode(); \
  cpu.p.N = (operand & 0x80) != 0; \
  cpu.p.V = (operand & 0x40) != 0; \
  cpu.p.Z = (operand & cpu.a) == 0; \
  return 0; \
}

#define PLA [](CPU6502& cpu) {\
  cpu.a = cpu.check_zn_flags(cpu.pop()); \
  return 0; \
}

#define PLP [](CPU6502& cpu) {\
  cpu.pop_flags(); \
  return 0; \
}

#define SE(reg, v) [](CPU6502& cpu) {\
  cpu.p.reg = v; \
  return 0; \
}

#define TXS [](CPU6502& cpu) {\
  cpu.sp = cpu.x; \
  return 0; \
}

#define T__(src, dst) [](CPU6502& cpu) {\
  cpu.check_zn_flags(cpu.dst = cpu.src); \
  return 0; \
}
