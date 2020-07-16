#pragma once

#define OP(op, mode) [](CPU5A22& cpu) {\
  word operand = cpu.deref_##mode(!cpu.p.m); \
  cpu.store(cpu.a, cpu.check_zn_flags(cpu.a.w op operand, !cpu.p.m), !cpu.p.m); \
  return cpu.observe_crossed_page(); \
}

#define GEN_A(body_macro_name) [](CPU5A22& cpu) {\
  word operand = cpu.a; \
  body_macro_name(operand, !cpu.p.m); \
  cpu.store(cpu.a, operand, !cpu.p.m); \
  return cpu.observe_crossed_page(); \
}

#define GEN(body_macro_name, mode) [](CPU5A22& cpu) {\
  CPU5A22::pc_t pc = cpu.pc; \
  dword addr = cpu.addr_##mode(); \
  cpu.pc = pc; \
  word operand = cpu.deref_##mode(!cpu.p.m); \
  body_macro_name(operand, !cpu.p.m); \
  if (cpu.p.m) \
    cpu.write(addr, operand); \
  else { \
    cpu.write(addr, operand & 0xff); \
    cpu.write(addr + 1, operand >> 8); \
  } \
  return cpu.observe_crossed_page(); \
}

// CAUTION: Since we use a 16-bit variable _operand_ even if cpu.deref__##mode() results
// in an 8-bit value, we need to mask after bitwise negating the entire thing.
#define ADC_GEN(mode, unary_op) [](CPU5A22& cpu) {\
  word operand = (unary_op cpu.deref_##mode(!cpu.p.m)) & (cpu.p.m ? 0xff : 0xffff); \
  word acc = cpu.a; \
  int addend = static_cast<int>(operand) + cpu.p.C; \
  int result = (cpu.a & (cpu.p.m ? 0xff : 0xffff)) + addend; \
  cpu.p.C = result > (cpu.p.m ? 0xff : 0xffff); \
  cpu.store(cpu.a, cpu.check_zn_flags(static_cast<word>(result), !cpu.p.m), !cpu.p.m); \
  cpu.p.V = ((acc ^ cpu.a) & (operand ^ cpu.a) & (cpu.p.m ? 0x80 : 0x8000)) != 0; \
  return cpu.observe_crossed_page(); \
}

#define ADC(mode) ADC_GEN(mode, +)
#define SBC(mode) ADC_GEN(mode, ~)

#define ASL_BODY(operand, op16) \
  bool msb = (operand & (op16 ? 0x8000 : 0x80)) != 0; \
  operand = cpu.check_zn_flags((operand << 1) & (op16 ? 0xfffe : 0xfe), op16); \
  cpu.p.C = msb;

// TODO: Z needs to respect 8/16-bit
#define LSR_BODY(operand, op16) \
  bool lsb = operand & 0b1; \
  operand = (operand >> 1) & (op16 ? 0x7fff : 0x7f); \
  cpu.p.C = lsb; \
  cpu.p.Z = ((operand & (op16 ? 0xffff : 0xff)) == 0); \
  cpu.p.N = 0;

#define ROL_BODY(operand, op16) \
  bool msb = (operand & (cpu.p.m ? 0x80 : 0x8000)) != 0; \
  operand = cpu.check_zn_flags(cpu.p.C | (operand << 1), op16); \
  cpu.p.C = msb;

#define ROR_BODY(operand, op16) \
  bool lsb = operand & 0b1; \
  operand = cpu.check_zn_flags((cpu.p.C << (op16 ? 15 : 7)) | (operand >> 1), op16); \
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
  if (cpu.p.m) { \
    cpu.a.l = (cpu.a.l >> 1) & 0x7f; \
  } else { \
    cpu.a.w = (cpu.a.w >> 1) & 0x7fff; \
  } \
  cpu.p.C = lsb; \
  cpu.p.Z = ((cpu.a & (cpu.p.m ? 0xff : 0xffff)) == 0); \
  cpu.p.N = 0; \
  return cpu.observe_crossed_page(); \
}

#define STA(mode) [](CPU5A22& cpu) {\
  dword addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.a.l); \
  if (!cpu.p.m) { \
    cpu.write(addr + 1, cpu.a.h); \
  } \
  return cpu.observe_crossed_page(); \
}

#define ST(reg, mode) [](CPU5A22& cpu) {\
  dword addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.reg.l); \
  if (!cpu.p.x) { \
    cpu.write(addr + 1, cpu.reg.h); \
  } \
  return cpu.observe_crossed_page(); \
}

#define STZ(mode) [](CPU5A22& cpu) { \
  dword addr = cpu.addr_##mode(); \
  cpu.write(addr, 0x0); \
  if (!cpu.p.m) { \
    cpu.write(addr + 1, 0x0); \
  } \
  return -cpu.p.m; \
}

#define LDA(mode) [](CPU5A22& cpu) {\
  word operand = cpu.deref_##mode(!cpu.p.m); \
  cpu.store(cpu.a, cpu.check_zn_flags(operand, !cpu.p.m), !cpu.p.m); \
  return cpu.observe_crossed_page(); \
}

#define LD(reg, mode) [](CPU5A22& cpu) {\
  word operand = cpu.deref_##mode(!cpu.p.x); \
  cpu.store(cpu.reg, cpu.check_zn_flags(operand, !cpu.p.x), !cpu.p.x); \
  return cpu.observe_crossed_page(); \
}

#define CMP_GEN(target_dual, mode, addt_cycles, op16_test) [](CPU5A22& cpu) {\
  word val = cpu.deref_##mode(op16_test); \
  word target = op16_test ? target_dual.w : target_dual.l; \
  cpu.check_zn_flags(target - val, op16_test); \
  cpu.p.C = target >= val; \
  return addt_cycles; \
}

#define CMP(mode) CMP_GEN(cpu.a, mode, cpu.observe_crossed_page(), (!cpu.p.m))
#define CP(reg, mode) CMP_GEN(cpu.reg, mode, 0, (!cpu.p.x))

// TODO: needs to respect x,m flags
#define INC_GEN(mode, increment) [](CPU5A22& cpu) {\
  CPU5A22::pc_t pc = cpu.pc; \
  dword addr = cpu.addr_##mode(); \
  cpu.pc = pc; \
  word operand = cpu.deref_##mode(!cpu.p.m); \
  operand = cpu.check_zn_flags((operand + increment) & (cpu.p.m ? 0xff : 0xffff), !cpu.p.m); \
  if (cpu.p.m) \
    cpu.write(addr, operand); \
  else { \
    cpu.write(addr, operand & 0xff); \
    cpu.write(addr + 1, operand >> 8); \
  } \
  return cpu.observe_crossed_page(); \
}

#define DEC(mode) INC_GEN(mode, -1)
#define INC(mode) INC_GEN(mode, +1)

#define IN_A [](CPU5A22& cpu) {\
  cpu.store(cpu.a, cpu.check_zn_flags( \
    (cpu.a.w + 1) & (cpu.p.m ? 0xff : 0xffff), !cpu.p.m), !cpu.p.m); \
  return 0; \
}

#define DE_A [](CPU5A22& cpu) {\
  cpu.store(cpu.a, cpu.check_zn_flags( \
    (cpu.a.w - 1) & (cpu.p.m ? 0xff : 0xffff), !cpu.p.m), !cpu.p.m); \
  return 0; \
}

// TODO: needs to respect x,m flags
// TODO: if the register is in 8-bit mode and we increment, should we wrap?
#define IN(reg) [](CPU5A22& cpu) {\
  cpu.store(cpu.reg, cpu.check_zn_flags( \
    (cpu.reg.w + 1) & (cpu.p.x ? 0xff : 0xffff), !cpu.p.x), !cpu.p.x); \
  return 0; \
}

#define DE(reg) [](CPU5A22& cpu) {\
  cpu.store(cpu.reg, cpu.check_zn_flags( \
    (cpu.reg.w - 1) & (cpu.p.x ? 0xff : 0xffff), !cpu.p.x), !cpu.p.x); \
  return 0; \
}

#define BIT(mode) [](CPU5A22& cpu) {\
  word operand = cpu.deref_##mode(!cpu.p.m); \
  cpu.p.N = (operand & (cpu.p.m ? 0x80 : 0x8000)) != 0; \
  cpu.p.V = (operand & (cpu.p.m ? 0x40 : 0x4000)) != 0; \
  cpu.p.Z = (operand & (cpu.a & (cpu.p.m ? 0xff : 0xffff))) == 0; \
  return 0; \
}

#define JSR [](CPU5A22& cpu) {\
  cpu.push_word(cpu.pc.addr + 1); \
  cpu.pc.c = cpu.read_word(); \
  return 0; \
}

// TODO:
#define JSL [](CPU5A22& cpu) {\
  cpu.jsl(); \
  return 0; \
}

// TODO:
#define JSR_abs_plus_x_indirect [](CPU5A22& cpu) {\
  cpu.push_word(cpu.pc.addr + 1); \
  auto zp_base = cpu.read_word(); \
  dword ptr = (cpu.pc.b << 16) | (zp_base + cpu.x); \
  dword addr = (cpu.pc.b << 16) | cpu.read_word(ptr); \
  cpu.pc.c = addr; \
  return 0; \
}

#define RTI [](CPU5A22& cpu) {\
  cpu.rti(); return 0; \
}

#define BRL [](CPU5A22& cpu) { \
  return cpu.branch_with_far_offset(); \
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

#define BRK        [](CPU5A22& cpu) { cpu.brk();                  return int(cpu.native()); }
#define JMP(mode)  [](CPU5A22& cpu) { cpu.pc.addr = cpu.addr_##mode(); return 0; }
#define JMPS(mode)  [](CPU5A22& cpu) { cpu.pc.addr = cpu.addr_same_bank_##mode(); return 0; }

#define PHP        [](CPU5A22& cpu) { cpu.push(cpu.p.reg); return 0; }
#define PLP        [](CPU5A22& cpu) { cpu.pop_flags();            return 0; }

#define PHA        [](CPU5A22& cpu) { if (cpu.p.m) cpu.push(cpu.a); else cpu.push_word(cpu.a); return 0; }
#define PLA        [](CPU5A22& cpu) { \
  cpu.store(cpu.a, cpu.check_zn_flags(cpu.p.m ? cpu.pop() : cpu.pop_word(), !cpu.p.m), !cpu.p.m); \
  return 0; \
}
#define PHX [](CPU5A22& cpu) { \
  if (cpu.p.x) { \
    cpu.push(cpu.x); \
  } else { \
    cpu.push_word(cpu.x); \
  } \
  return 0; \
}
#define PLX [](CPU5A22& cpu) { cpu.store(cpu.x, cpu.check_zn_flags(cpu.p.x ? cpu.pop() : cpu.pop_word(), !cpu.p.x), !cpu.p.x); return 0; }
#define PHY [](CPU5A22& cpu) { \
  if (cpu.p.x) { \
    cpu.push(cpu.y); \
  } else { \
    cpu.push_word(cpu.y); \
  } \
  return 0; \
}
#define PLY [](CPU5A22& cpu) { cpu.store(cpu.y, cpu.check_zn_flags(cpu.p.x ? cpu.pop() : cpu.pop_word(), !cpu.p.x), !cpu.p.x); return 0; }

#define RTS        [](CPU5A22& cpu) { cpu.rts(); return 0; }
#define RTL        [](CPU5A22& cpu) { cpu.rtl(); return 0; }


#define SE(reg, v) [](CPU5A22& cpu) { cpu.p.reg = v;              return 0; }

#define PEA [](CPU5A22& cpu) { \
  cpu.push_word(cpu.read_word()); \
  return 0; \
}
#define PEI [](CPU5A22& cpu) { \
  word word_addr = cpu.addr_zpg(); \
  auto addr = cpu.read_word(word_addr); \
  cpu.push_word(addr); \
  return 0; \
}

#define PER [](CPU5A22& cpu) { \
  sword offset = static_cast<sword>(cpu.read_word()); \
  cpu.push_word(cpu.pc.addr + offset); \
  return 0; \
}

#define PHB [](CPU5A22& cpu) { cpu.push(cpu.db); return 0; }
#define PLB [](CPU5A22& cpu) { cpu.check_zn_flags(cpu.db = cpu.pop(), false); return 0; }
#define PHD [](CPU5A22& cpu) { cpu.push_word(cpu.dp); return 0; }
#define PLD [](CPU5A22& cpu) { cpu.check_zn_flags(cpu.dp = cpu.pop_word(), true); return 0; }
#define PHK [](CPU5A22& cpu) { cpu.push(cpu.pc.b); return 0; }
#define WDM XX(1, 2)
#define MVP [](CPU5A22& cpu) { return cpu.mvp(); }
#define MVN [](CPU5A22& cpu) { return cpu.mvn(); }
#define WAI [](CPU5A22& cpu) { return 0; }
#define XBA [](CPU5A22& cpu) { byte tmp = cpu.a.h; cpu.a.h = cpu.a.l; cpu.check_zn_flags(cpu.a.l = tmp, false); return 0; }
#define XCE [](CPU5A22& cpu) { \
  bool tmp = cpu.p.C; cpu.p.C = cpu.e; cpu.e = tmp; \
  if (cpu.e) { \
    cpu.p.m = 1; \
    cpu.p.x = 1; \
    cpu.x.h = 0x0; \
    cpu.y.h = 0x0; \
    cpu.sp.h = 0x1; \
  } \
  return 0; \
}
#define STP [](CPU5A22& cpu) { cpu.stp(); return 0; }
#define COP [](CPU5A22& cpu) { cpu.cop(); return 0; }

// TODO: handle e flag
#define TCD [](CPU5A22& cpu) { \
  cpu.check_zn_flags(cpu.dp = cpu.a.w, true); \
  return 0; \
}
#define TDC [](CPU5A22& cpu) { \
  cpu.check_zn_flags(cpu.a.w = cpu.dp, true); \
  return 0; \
}
// TCS does not set any flags, unlike TCD, TDC, TSC
#define TCS [](CPU5A22& cpu) { \
  cpu.sp.w = cpu.a.w; \
  return 0; \
}
#define TSC [](CPU5A22& cpu) { \
  cpu.check_zn_flags(cpu.a.w = cpu.sp.w, true); \
  return 0; \
}

#define SEP [](CPU5A22& cpu) { \
  byte mask = cpu.read_byte(); \
  cpu.p.reg |= mask; \
  if (cpu.p.x) { \
    cpu.x.h = 0; cpu.y.h = 0; \
  } \
  return 0; \
}
#define REP [](CPU5A22& cpu) { \
  byte mask = cpu.read_byte(); \
  cpu.p.reg &= ~mask; \
  return 0; \
}

// TODO: no effect yet
// Exceptionally, these take 8- or 16-bit data according to the opcode (and not any processor
// flags such as e, x or m.)
#define TSB(mode) [](CPU5A22& cpu) { \
  CPU5A22::pc_t pc = cpu.pc; \
  dword addr = cpu.addr_##mode(); \
  cpu.pc = pc; \
  word operand = cpu.deref_##mode(!cpu.p.m); \
  if (cpu.p.m) { \
    cpu.p.Z = ((operand & cpu.a.l) & 0xff) == 0x0; \
    operand |= cpu.a.l; \
    cpu.write(addr, operand); \
  } else { \
    cpu.p.Z = (operand & cpu.a.w) == 0x0; \
    operand |= cpu.a.w; \
    cpu.write(addr, operand & 0xff); \
    cpu.write(addr + 1, operand >> 8); \
  } \
  return 0; \
}

#define TRB(mode) [](CPU5A22& cpu) { \
  CPU5A22::pc_t pc = cpu.pc; \
  dword addr = cpu.addr_##mode(); \
  cpu.pc = pc; \
  word operand = cpu.deref_##mode(!cpu.p.m); \
  if (cpu.p.m) { \
    cpu.p.Z = ((operand & cpu.a.l) & 0xff) == 0x0; \
    operand &= ~cpu.a.l; \
    cpu.write(addr, operand); \
  } else { \
    cpu.p.Z = (operand & cpu.a.w) == 0x0; \
    operand &= ~cpu.a.w; \
    cpu.write(addr, operand & 0xff); \
    cpu.write(addr + 1, operand >> 8); \
  } \
  return 0; \
}

#define BIT_NUM [](CPU5A22& cpu) { \
  word operand = cpu.deref_imm(!cpu.p.m); \
  cpu.check_zn_flags(cpu.a.w & operand, !cpu.p.m); \
  return 0; \
}

// The size of the _dst_ register determines whether this is an 8- or 16-bit operation.
// If the dest is 8 bits, then only 8 bits are transferred, etc.
// Note that the S register is always considered 16 bits wide.
#define T__(src, dst) [](CPU5A22& cpu) {\
  if (cpu.p.x) { \
    cpu.check_zn_flags(cpu.dst.l = cpu.src.l, !cpu.p.x); \
  } else { \
    cpu.check_zn_flags(cpu.dst = cpu.src, !cpu.p.x); \
  } \
  return 0; \
}
#define T_A(src) [](CPU5A22& cpu) {\
  if (cpu.p.m) { \
    cpu.check_zn_flags(cpu.a.l = cpu.src.l, !cpu.p.m); \
  } else { \
    cpu.check_zn_flags(cpu.a = cpu.src, !cpu.p.m); \
  } \
  return 0; \
}

#define TXS        [](CPU5A22& cpu) { cpu.sp = cpu.x;             return 0; }
