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
  word acc = cpu.a;                               \
  int result;                                     \
  if (!cpu.p.D) {                                                                      \
    int addend = static_cast<int>(operand) + cpu.p.C;                                 \
    result = (cpu.a & (cpu.p.m ? 0xff : 0xffff)) + addend;                         \
    cpu.p.C = result > (cpu.p.m ? 0xff : 0xffff); \
  } else {                                        \
    int addend = cpu.bcd_add(static_cast<int>(operand), cpu.p.C);                       \
    result = cpu.bcd_add(cpu.a & (cpu.p.m ? 0xff : 0xffff), addend);                \
    cpu.p.C = result > (cpu.p.m ? 0x99 : 0x9999); \
  }                                               \
  cpu.store(cpu.a, cpu.check_zn_flags(static_cast<word>(result), !cpu.p.m), !cpu.p.m); \
  if (!cpu.p.D) {                                 \
    cpu.p.V = ((acc ^ cpu.a) & (operand ^ cpu.a) & (cpu.p.m ? 0x80 : 0x8000)) != 0;    \
  } else {                                                                             \
    cpu.p.V = result < -128 || result > 127;                                         \
  }                                               \
  return cpu.observe_crossed_page(); \
}

#define ADC(mode) ADC_GEN(mode, +)
#define SBC(mode) ADC_GEN(mode, ~)

#define ASL_BODY(operand, op16) \
  bool msb = (operand & (op16 ? 0x8000 : 0x80)) != 0; \
  operand = cpu.check_zn_flags((operand << 1) & (op16 ? 0xfffe : 0xfe), op16); \
  cpu.p.C = msb;

#define LSR_BODY(operand, op16) \
  bool lsb = operand & 0b1; \
  operand = (operand >> 1) & (op16 ? 0x7fff : 0x7f); \
  cpu.p.C = lsb; \
  cpu.check_z_flag(operand, op16); \
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
  cpu.check_z_flag(cpu.a, !cpu.p.m); \
  cpu.p.N = 0; \
  return cpu.observe_crossed_page(); \
}

#define ST_GEN(reg, mode, op16) [](CPU5A22& cpu) {\
  dword addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.reg.l); \
  if (op16) { \
    cpu.write(addr + 1, cpu.reg.h); \
  } \
  return cpu.observe_crossed_page(); \
}

#define STA(mode) ST_GEN(a, mode, !cpu.p.m)
#define ST(reg, mode) ST_GEN(reg, mode, !cpu.p.x)

#define STZ(mode) [](CPU5A22& cpu) { \
  dword addr = cpu.addr_##mode(); \
  cpu.write(addr, 0x0); \
  if (!cpu.p.m) { \
    cpu.write(addr + 1, 0x0); \
  } \
  return -cpu.p.m; \
}

#define LD_GEN(reg, mode, op16) [](CPU5A22& cpu) {\
  word operand = cpu.deref_##mode(op16); \
  cpu.store(cpu.reg, cpu.check_zn_flags(operand, op16), op16); \
  return cpu.observe_crossed_page(); \
}

#define LDA(mode) LD_GEN(a, mode, !cpu.p.m)
#define LD(reg, mode) LD_GEN(reg, mode, !cpu.p.x)

#define CMP_GEN(target_dual, mode, addt_cycles, op16_test) [](CPU5A22& cpu) {\
  word val = cpu.deref_##mode(op16_test); \
  word target = op16_test ? target_dual.w : target_dual.l; \
  cpu.check_zn_flags(target - val, op16_test); \
  cpu.p.C = target >= val; \
  return addt_cycles; \
}

#define CMP(mode) CMP_GEN(cpu.a, mode, cpu.observe_crossed_page(), (!cpu.p.m))
#define CP(reg, mode) CMP_GEN(cpu.reg, mode, 0, (!cpu.p.x))

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

#define IN_GEN(reg, op, op16) [](CPU5A22& cpu) {\
  cpu.store(cpu.reg, cpu.check_zn_flags( \
    (cpu.reg.w op 1) & (op16 ? 0xffff : 0xff), op16), op16); \
  return 0; \
}

#define IN_A IN_GEN(a, +, !cpu.p.m)
#define DE_A IN_GEN(a, -, !cpu.p.m)
#define IN(reg) IN_GEN(reg, +, !cpu.p.x)
#define DE(reg) IN_GEN(reg, -, !cpu.p.x)

#define BIT(mode) [](CPU5A22& cpu) {\
  word operand = cpu.deref_##mode(!cpu.p.m); \
  cpu.check_n_flag(operand, !cpu.p.m); \
  cpu.p.V = (operand & (cpu.p.m ? 0x40 : 0x4000)) != 0; \
  cpu.p.Z = (operand & (cpu.a & (cpu.p.m ? 0xff : 0xffff))) == 0; \
  return 0; \
}

#define JSR [](CPU5A22& cpu) {\
  cpu.push_word(cpu.pc.addr + 1); \
  cpu.pc.c = cpu.read_word(); \
  return 0; \
}

#define JSL [](CPU5A22& cpu) {\
  cpu.jsl(); \
  return 0; \
}

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

#define NOP        [](CPU5A22& cpu) { return 0; }
#define XX(len, n) [](CPU5A22& cpu) { cpu.pc.addr += len; return n; }
#define XXX        [](CPU5A22& cpu) { return 0; }

#define BRK        [](CPU5A22& cpu) { cpu.brk(); return int(cpu.native()); }
#define JMP(mode)  [](CPU5A22& cpu) { cpu.pc.addr = cpu.addr_##mode(); return 0; }
#define JMPS(mode)  [](CPU5A22& cpu) { cpu.pc.addr = cpu.addr_same_bank_##mode(); return 0; }

#define PH_(reg, op16) [](CPU5A22& cpu) { if (op16) cpu.push_word(cpu.reg); else cpu.push(cpu.reg); return 0; }
#define PL_(reg, op16) [](CPU5A22& cpu) { cpu.store(cpu.reg, cpu.check_zn_flags(op16 ? cpu.pop_word() : cpu.pop(), op16), op16); return 0; }

#define RTS        [](CPU5A22& cpu) { cpu.rts(); return 0; }
#define RTL        [](CPU5A22& cpu) { cpu.rtl(); return 0; }

#define SE(reg, v) [](CPU5A22& cpu) { cpu.p.reg = v; return 0; }

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

#define PHP [](CPU5A22& cpu) { cpu.push(cpu.p.reg); return 0; }
#define PLP [](CPU5A22& cpu) { cpu.pop_flags(); return 0; }
#define PHB [](CPU5A22& cpu) { cpu.push(cpu.db); return 0; }
#define PLB [](CPU5A22& cpu) { cpu.check_zn_flags(cpu.db = cpu.pop(), false); return 0; }
#define PHD [](CPU5A22& cpu) { cpu.push_word(cpu.dp); return 0; }
#define PLD [](CPU5A22& cpu) { cpu.check_zn_flags(cpu.dp = cpu.pop_word(), true); return 0; }
#define PHK [](CPU5A22& cpu) { cpu.push(cpu.pc.b); return 0; }
#define WDM XX(1, 2)
#define MVP [](CPU5A22& cpu) { return cpu.mv_(true); }
#define MVN [](CPU5A22& cpu) { return cpu.mv_(false); }
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

// Exceptionally, these take 8- or 16-bit data according to the opcode (and not any processor
// flags such as e, x or m.)
#define T_B(mode, op, unary_op) [](CPU5A22& cpu) { \
  CPU5A22::pc_t pc = cpu.pc; \
  dword addr = cpu.addr_##mode(); \
  cpu.pc = pc; \
  word operand = cpu.deref_##mode(!cpu.p.m); \
  if (cpu.p.m) { \
    cpu.p.Z = ((operand & cpu.a.l) & 0xff) == 0x0; \
    operand = operand op unary_op cpu.a.l; \
    cpu.write(addr, operand); \
  } else { \
    cpu.p.Z = (operand & cpu.a.w) == 0x0; \
    operand = operand op unary_op cpu.a.w; \
    cpu.write(addr, operand & 0xff); \
    cpu.write(addr + 1, operand >> 8); \
  } \
  return 0; \
}

#define TSB(mode) T_B(mode, |, +)
#define TRB(mode) T_B(mode, &, ~)

#define BIT_NUM [](CPU5A22& cpu) { \
  word operand = cpu.deref_imm(!cpu.p.m); \
  cpu.check_zn_flags(cpu.a.w & operand, !cpu.p.m); \
  return 0; \
}

// The size of the _dst_ register determines whether this is an 8- or 16-bit operation.
// If the dest is 8 bits, then only 8 bits are transferred, etc.
// Note that the S register is always considered 16 bits wide.
#define T__GEN(src, dst, op16) [](CPU5A22& cpu) {\
  if (!op16) { \
    cpu.check_zn_flags(cpu.dst.l = cpu.src.l, op16); \
  } else { \
    cpu.check_zn_flags(cpu.dst = cpu.src, op16); \
  } \
  return 0; \
}
#define T__(src, dst) T__GEN(src, dst, !cpu.p.x)
#define T_A(src)      T__GEN(src, a, !cpu.p.m)

#define TXS [](CPU5A22& cpu) { cpu.sp = cpu.x; return 0; }
