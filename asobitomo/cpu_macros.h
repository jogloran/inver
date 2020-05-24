#pragma once

#include "types.h"
#include "flags.h"
#include "cpu.h"

#define NOOP [](CPU&) {}

#define LD_WORD_d16(hi, lo) [](CPU& cpu) { \
  cpu.hi = cpu.mmu[cpu.pc + 1]; \
  cpu.lo = cpu.mmu[cpu.pc]; \
  cpu.pc += 2; \
}

#define LD_WWORD_d16(wword) [](CPU& cpu) { \
  cpu.wword = cpu.get_word(); \
}

#define LD_ADDR_REG(hi, lo, reg) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.hi, cpu.lo); \
  cpu.mmu.set(loc, cpu.reg); \
}

#define LD_LOC_SP() [](CPU& cpu) { \
  word loc = cpu.get_word(); \
  cpu.mmu.set(loc, cpu.sp & 0xff); \
  cpu.mmu.set(loc + 1, cpu.sp >> 8); \
}

#define INC_WORD(hi, lo) [](CPU& cpu) { \
  word bc = cpu.get_word(cpu.hi, cpu.lo); \
  ++bc; \
  cpu.hi = bc >> 8; \
  cpu.lo = bc & 0xff; \
}

#define INC_WWORD(wword) [](CPU& cpu) { \
  ++cpu.wword; \
}

#define DEC_WORD(hi, lo) [](CPU& cpu) { \
  word bc = cpu.get_word(cpu.hi, cpu.lo); \
  --bc; \
  cpu.hi = bc >> 8; \
  cpu.lo = bc & 0xff; \
}

#define DEC_WWORD(wword) [](CPU& cpu) { \
  --cpu.wword; \
}

#define INC_REG(byte) [](CPU& cpu) { \
  if ((cpu.byte & 0xf) == 0xf) { \
    cpu.set_flags(Hf); \
  } else { \
    cpu.unset_flags(Hf); \
  } \
  ++cpu.byte; \
  cpu.unset_flags(Nf); \
  cpu.check_zero(cpu.byte); \
}

#define DEC_REG(byte) [](CPU& cpu) { \
  if ((cpu.byte & 0xf) == 0) { \
    cpu.set_flags(Hf); \
  } else { \
    cpu.unset_flags(Hf); \
  } \
  --cpu.byte; \
  cpu.set_flags(Nf); \
  cpu.check_zero(cpu.byte); \
}

#define INC_ADDR(hi, lo) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.hi, cpu.lo); \
  cpu.check_half_carry(cpu.mmu[loc], 1); \
  cpu.mmu.set(loc, cpu.mmu[loc] + 1); \
  cpu.check_zero(cpu.mmu[loc]); \
  cpu.unset_flags(Nf); \
}

#define DEC_ADDR(hi, lo) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.hi, cpu.lo); \
  cpu.check_half_carry_sub(cpu.mmu[loc], 1); \
  cpu.mmu.set(loc, cpu.mmu[loc] - 1); \
  cpu.check_zero(cpu.mmu[loc]); \
  cpu.set_flags(Nf); \
}

#define LD_REG_d8(reg) [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  cpu.reg = d8; \
}

#define LD_ADDR_d8(hi, lo) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.hi, cpu.lo); \
  byte d8 = cpu.get_byte(); \
  cpu.mmu.set(loc, d8); \
}

#define RLCA() [](CPU& cpu) { \
  byte hibit = cpu.check_carry_if_hibit_set(cpu.a); \
  cpu.a <<= 1; \
  cpu.a |= hibit; \
  cpu.unset_flags(Nf | Hf | Zf); \
}

#define RRCA() [](CPU& cpu) { \
  byte lobit = cpu.check_carry_if_lobit_set(cpu.a); \
  cpu.a >>= 1; \
  cpu.a |= lobit << 7; \
  cpu.unset_flags(Nf | Hf | Zf); \
}

 #define RLA() [](CPU& cpu) { \
   byte carry = cpu.C(); \
   cpu.check_carry_if_hibit_set(cpu.a); \
   cpu.a <<= 1; \
   cpu.a |= carry; \
   cpu.unset_flags(Nf | Hf | Zf); \
 }

#define RRA() [](CPU& cpu) { \
   byte carry = cpu.C(); \
   cpu.check_carry_if_lobit_set(cpu.a); \
   cpu.a >>= 1; \
   cpu.a |= carry << 7; \
   cpu.unset_flags(Nf | Hf | Zf); \
}

#define ADD_WORD_WORD(hi1, lo1, hi2, lo2) [](CPU& cpu) { \
  word hl = cpu.get_word(cpu.hi1, cpu.lo1); \
  word bc = cpu.get_word(cpu.hi2, cpu.lo2); \
  cpu.check_half_carry_word(hl, bc); \
  uint32_t result = static_cast<uint32_t>(hl) + static_cast<uint32_t>(bc); \
  hl = static_cast<word>(result); \
  cpu.hi1 = hl >> 8; \
  cpu.lo1 = hl & 0xff; \
  cpu.unset_flags(Nf); \
  cpu.check_carry_word(result); \
}

#define ADD_WORD_WWORD(hi, lo, wword) [](CPU& cpu) { \
  word hl = cpu.get_word(cpu.hi, cpu.lo); \
  word bc = cpu.wword; \
  cpu.check_half_carry_word(hl, bc); \
  uint32_t result = static_cast<uint32_t>(hl) + static_cast<uint32_t>(bc); \
  hl = static_cast<word>(result); \
  cpu.hi = hl >> 8; \
  cpu.lo = hl & 0xff; \
  cpu.unset_flags(Nf); \
  cpu.check_carry_word(result); \
}

#define LD_REG_LOC(reg, hi, lo) [](CPU& cpu) { \
  word bc = cpu.get_word(cpu.hi, cpu.lo); \
  cpu.reg = cpu.mmu[bc]; \
}

#define LD_REG_REG8(dest) LD_REG_REG8_helper(dest, b), \
LD_REG_REG8_helper(dest, c), \
LD_REG_REG8_helper(dest, d), \
LD_REG_REG8_helper(dest, e), \
LD_REG_REG8_helper(dest, h), \
LD_REG_REG8_helper(dest, l), \
LD_REG_REG8_HL_LOC_helper(dest), \
LD_REG_REG8_helper(dest, a)

#define LD_REG_REG8_helper(dest, src) [](CPU& cpu) { \
  cpu.dest = cpu.src; \
}

#define LD_REG_REG8_HL_LOC_helper(dest) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  cpu.dest = cpu.mmu[loc]; \
}

#define HALT() [](CPU& cpu) { \
  cpu.halt(); \
}

#define LD_HL_SPECIAL() LD_HL_SPECIAL_helper(b), \
LD_HL_SPECIAL_helper(c), \
LD_HL_SPECIAL_helper(d), \
LD_HL_SPECIAL_helper(e), \
LD_HL_SPECIAL_helper(h), \
LD_HL_SPECIAL_helper(l), \
HALT(), \
LD_HL_SPECIAL_helper(a)

#define LD_HL_SPECIAL_helper(src) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  cpu.mmu.set(loc, cpu.src); \
}

#define JR_COND_r8(cond) [](CPU& cpu) { \
  if ((cond)) { \
    int8_t r8 = cpu.get_sbyte(); \
    cpu.pc = cpu.pc + r8; \
    cpu.cycles += 4; \
  } else { \
    cpu.pc += 1; \
  } \
}

#define CPL() [](CPU& cpu) { \
  cpu.a = ~cpu.a; \
  cpu.set_flags(Nf | Hf); \
}

#define SCF() [](CPU& cpu) { \
  cpu.set_flags(Cf); \
  cpu.unset_flags(Nf | Hf); \
}

#define CCF() [](CPU& cpu) { \
  cpu.toggle_flags(Cf); \
  cpu.unset_flags(Nf | Hf); \
}

#define AND_A8() AND_A8_HELPER(b), \
AND_A8_HELPER(c), \
AND_A8_HELPER(d), \
AND_A8_HELPER(e), \
AND_A8_HELPER(h), \
AND_A8_HELPER(l), \
AND_A8_HL_LOC_HELPER(), \
AND_A8_HELPER(a)

#define AND_A8_HELPER(src) [](CPU& cpu) { \
  cpu.a = cpu.a & cpu.src; \
  cpu.unset_flags(Nf | Cf); \
  cpu.set_flags(Hf); \
  cpu.check_zero(cpu.a); \
}

#define AND_A8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  cpu.a = cpu.a & cpu.mmu[loc]; \
  cpu.unset_flags(Nf | Cf); \
  cpu.set_flags(Hf); \
  cpu.check_zero(cpu.a); \
}

#define SUB_A8() SUB_A8_HELPER(b), \
SUB_A8_HELPER(c), \
SUB_A8_HELPER(d), \
SUB_A8_HELPER(e), \
SUB_A8_HELPER(h), \
SUB_A8_HELPER(l), \
SUB_A8_HL_LOC_HELPER(), \
SUB_A8_HELPER(a)

#define SUB_A8_HELPER(src) [](CPU& cpu) { \
  int result = static_cast<int>(cpu.a) - cpu.src; \
  cpu.check_carry(result); \
  cpu.check_half_carry_sub(cpu.a, cpu.src); \
  cpu.a = static_cast<byte>(result); \
  cpu.set_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define SUB_A8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  int result = static_cast<int>(cpu.a) - cpu.mmu[loc]; \
  cpu.check_carry(result); \
  cpu.check_half_carry_sub(cpu.a, cpu.mmu[loc]); \
  cpu.a = static_cast<byte>(result); \
  cpu.set_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define ADD_A8_HELPER(src) [](CPU& cpu) { \
  word result = cpu.a + cpu.src; \
  cpu.check_carry(result); \
  cpu.check_half_carry(cpu.a, cpu.src); \
  cpu.a = static_cast<byte>(result); \
  cpu.unset_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define ADD_A8() ADD_A8_HELPER(b), \
ADD_A8_HELPER(c), \
ADD_A8_HELPER(d), \
ADD_A8_HELPER(e), \
ADD_A8_HELPER(h), \
ADD_A8_HELPER(l), \
ADD_A8_HL_LOC_HELPER(), \
ADD_A8_HELPER(a)

#define ADD_A8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  word result = cpu.a + cpu.mmu[loc]; \
  cpu.check_carry(result); \
  cpu.check_half_carry(cpu.a, cpu.mmu[loc]); \
  cpu.a = static_cast<byte>(result); \
  cpu.unset_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define ADC_A8_HELPER(src) [](CPU& cpu) { \
  word result = cpu.a + cpu.src + cpu.C(); \
  cpu.check_half_carry(cpu.a, cpu.src, cpu.C()); \
  cpu.check_carry(result); \
  cpu.a = static_cast<byte>(result); \
  cpu.unset_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define ADC_A8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  word result = cpu.a + cpu.mmu[loc] + cpu.C(); \
  cpu.check_half_carry(cpu.a, cpu.mmu[loc], cpu.C()); \
  cpu.check_carry(result); \
  cpu.a = static_cast<byte>(result); \
  cpu.unset_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define ADC_A8() ADC_A8_HELPER(b), \
ADC_A8_HELPER(c), \
ADC_A8_HELPER(d), \
ADC_A8_HELPER(e), \
ADC_A8_HELPER(h), \
ADC_A8_HELPER(l), \
ADC_A8_HL_LOC_HELPER(), \
ADC_A8_HELPER(a)

#define GEN8_HELPER(op, src) [](CPU& cpu) { \
  cpu.a = cpu.a op cpu.src; \
  cpu.unset_flags(Nf | Hf | Cf); \
  cpu.check_zero(cpu.a); \
}

#define GEN8_HL_LOC_HELPER(op) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  cpu.a = cpu.a op cpu.mmu[loc]; \
  cpu.unset_flags(Nf | Hf | Cf); \
  cpu.check_zero(cpu.a); \
}

#define GEN8(op) GEN8_HELPER(op, b), \
GEN8_HELPER(op, c), \
GEN8_HELPER(op, d), \
GEN8_HELPER(op, e), \
GEN8_HELPER(op, h), \
GEN8_HELPER(op, l), \
GEN8_HL_LOC_HELPER(op), \
GEN8_HELPER(op, a)

#define SBC_A8_HELPER(src) [](CPU& cpu) { \
  int result = static_cast<int>(cpu.a) - cpu.src - cpu.C(); \
  cpu.check_half_carry_sub(cpu.a, cpu.src, cpu.C()); \
  cpu.check_carry(result); \
  cpu.a = static_cast<byte>(result); \
  cpu.set_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define SBC_A8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  int result = static_cast<int>(cpu.a) - cpu.mmu[loc] - cpu.C(); \
  cpu.check_half_carry_sub(cpu.a, cpu.mmu[loc], cpu.C()); \
  cpu.check_carry(result); \
  cpu.a = static_cast<byte>(result); \
  cpu.set_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define SBC_A8() SBC_A8_HELPER(b), \
SBC_A8_HELPER(c), \
SBC_A8_HELPER(d), \
SBC_A8_HELPER(e), \
SBC_A8_HELPER(h), \
SBC_A8_HELPER(l), \
SBC_A8_HL_LOC_HELPER(), \
SBC_A8_HELPER(a)

#define CP8_HELPER(src) [](CPU& cpu) { \
  int result = static_cast<int>(cpu.a) - cpu.src; \
  if (result < 0) { \
    cpu.set_flags(Cf); \
  } else { \
    cpu.unset_flags(Cf); \
  } \
  cpu.check_half_carry_sub(cpu.a, cpu.src); \
  cpu.set_flags(Nf); \
  cpu.check_zero(result); \
}

#define CP8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  int result = static_cast<int>(cpu.a) - cpu.mmu[loc]; \
  cpu.check_carry(result); \
  cpu.check_half_carry_sub(cpu.a, cpu.mmu[loc]); \
  cpu.set_flags(Nf); \
  cpu.check_zero(result); \
}

#define CP8() CP8_HELPER(b), \
CP8_HELPER(c), \
CP8_HELPER(d), \
CP8_HELPER(e), \
CP8_HELPER(h), \
CP8_HELPER(l), \
CP8_HL_LOC_HELPER(), \
CP8_HELPER(a)

#define RET_COND(cond) [](CPU& cpu) { \
  if (cond) { \
    word loc = cpu.pop_word(); \
    cpu.pc = loc; \
    cpu.cycles += 12; \
  } \
}

#define POP_WORD(hi, lo) [](CPU& cpu) { \
  cpu.lo = cpu.mmu[cpu.sp]; \
  cpu.hi = cpu.mmu[cpu.sp + 1]; \
  cpu.sp += 2; \
}

#define POP_AF() [](CPU& cpu) { \
  cpu.f = cpu.mmu[cpu.sp] & 0xf0; \
  cpu.a = cpu.mmu[cpu.sp + 1]; \
  cpu.sp += 2; \
}

#define PUSH_WORD(hi, lo) [](CPU& cpu) { \
  cpu.push_word(cpu.hi, cpu.lo); \
}

#define JP_COND_a16(cond) [](CPU& cpu) { \
  if (cond) { \
    word a16 = cpu.get_word(); \
    cpu.pc = a16; \
    cpu.cycles += 4; \
  } else { \
    cpu.pc += 2; \
  } \
}

#define RST(addr) [](CPU& cpu) { \
  cpu.push_word(cpu.pc); \
  cpu.pc = addr; \
}

#define CALL_COND_a16(cond) [](CPU& cpu) { \
  word a16 = cpu.get_word(); \
  if (cond) { \
    cpu.push_word(cpu.pc); \
    cpu.pc = a16; \
    cpu.cycles += 12; \
  } \
}

#define ADD_A_d8() [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  int result = static_cast<int>(cpu.a) + d8; \
  cpu.check_half_carry(cpu.a, d8); \
  cpu.a = static_cast<byte>(result); \
  cpu.check_zero(cpu.a); \
  cpu.unset_flags(Nf); \
  cpu.check_carry(result); \
}

#define ADC_A_d8() [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  word result = cpu.a + d8 + cpu.C(); \
  cpu.check_half_carry(cpu.a, d8, cpu.C()); \
  cpu.check_carry(result); \
  cpu.a = static_cast<byte>(result); \
  cpu.unset_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define SUB_A_d8() [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  int result = static_cast<int>(cpu.a) - d8; \
  cpu.check_half_carry_sub(cpu.a, d8); \
  cpu.a = static_cast<byte>(result); \
  cpu.set_flags(Nf); \
  cpu.check_zero(cpu.a); \
  cpu.check_carry(result); \
}

#define SBC_A_d8() [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  int result = (static_cast<int>(cpu.a) - d8 - cpu.C()); \
  cpu.check_half_carry_sub(cpu.a, d8, cpu.C()); \
  cpu.check_carry(result); \
  cpu.a = static_cast<byte>(result); \
  cpu.set_flags(Nf); \
  cpu.check_zero(cpu.a); \
}

#define RETI() [](CPU& cpu) { \
  word loc = cpu.pop_word(); \
  cpu.pc = loc; \
  cpu.enable_interrupts(); \
}

#define AND_d8() [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  cpu.a = cpu.a & d8; \
  cpu.check_zero(cpu.a); \
  cpu.set_flags(Hf); \
  cpu.unset_flags(Nf | Cf); \
}

#define OP_d8(op) [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  cpu.a = cpu.a op d8; \
  cpu.check_zero(cpu.a); \
  cpu.unset_flags(Nf | Hf | Cf); \
}

#define CP_d8() [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  int result = static_cast<int>(cpu.a) - d8; \
  cpu.check_half_carry_sub(cpu.a, d8); \
  cpu.set_flags(Nf); \
  cpu.check_zero(result); \
  cpu.check_carry(result); \
}

#define JP_HL() [](CPU& cpu) { \
  word hl = cpu.get_word(cpu.h, cpu.l); \
  cpu.pc = hl; \
}

#define LD_SP_HL() [](CPU& cpu) { \
  word hl = cpu.get_word(cpu.h, cpu.l); \
  cpu.sp = hl; \
}

#define LD_A_a16() [](CPU& cpu) { \
  word a16 = cpu.get_word(); \
  cpu.a = cpu.mmu[a16]; \
}

#define LD_a16_A() [](CPU& cpu) { \
  word a16 = cpu.get_word(); \
  cpu.mmu.set(a16, cpu.a); \
}

#define ADD_SP_r8() [](CPU& cpu) { \
  int8_t r8 = cpu.get_byte(); \
  cpu.check_half_carry(cpu.sp & 0xff, r8); \
  /* pc-1 because get_byte advances pc */ \
  word unsigned_result = static_cast<word>(cpu.sp & 0xff) + static_cast<word>(cpu.mmu[cpu.pc-1]); \
  cpu.check_carry(unsigned_result); \
  cpu.sp = static_cast<int>(cpu.sp) + r8; \
  cpu.unset_flags(Zf | Nf); \
}

#define LD_HL_SP_plus_r8() [](CPU& cpu) { \
  int8_t r8 = cpu.get_byte(); \
  cpu.check_half_carry(cpu.sp & 0xff, r8); \
  /* pc-1 because get_byte advances pc */ \
  word unsigned_result = static_cast<word>(cpu.sp & 0xff) + static_cast<word>(cpu.mmu[cpu.pc - 1]); \
  cpu.check_carry(unsigned_result); \
  int hl = static_cast<int>(cpu.sp) + r8; \
  cpu.h = hl >> 8; \
  cpu.l = hl & 0xff; \
  cpu.unset_flags(Zf | Nf); \
}

#define LD_LOC_REG_AUG(op, hi, lo, reg) [](CPU& cpu) { \
  word hl = cpu.get_word(cpu.hi, cpu.lo); \
  cpu.mmu.set(hl, cpu.reg); \
  hl = hl op 1; \
  cpu.hi = hl >> 8; \
  cpu.lo = hl & 0xff; \
}

#define LDH_A_a8() [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  cpu.a = cpu.mmu[0xff00 + d8]; \
}

#define LDH_a8_A() [](CPU& cpu) { \
  byte d8 = cpu.get_byte(); \
  cpu.mmu.set(0xff00 + d8, cpu.a); \
}

#define LDH_ADDR_A(reg) [](CPU& cpu) { \
  cpu.mmu.set(0xff00 + cpu.reg, cpu.a); \
}

#define LDH_A_ADDR(reg) [](CPU& cpu) { \
  cpu.a = cpu.mmu[0xff00 + cpu.reg]; \
}

#define LD_REG_LOC_AUG(reg, op, hi, lo) [](CPU& cpu) { \
  word hl = cpu.get_word(cpu.hi, cpu.lo); \
  cpu.reg = cpu.mmu[hl]; \
  hl = hl op 1; \
  cpu.hi = hl >> 8; \
  cpu.lo = hl & 0xff; \
}

#define INVALID() [](CPU& cpu) {}
////  throw std::runtime_error("Invalid opcode"); \
//}

#define DI() [](CPU& cpu) { \
  cpu.disable_interrupts(); \
}

#define EI() [](CPU& cpu) { \
  cpu.enable_interrupts_next_instruction(); \
}

#define STOP() [](CPU& cpu) { \
  cpu.pc += 1; \
  cpu.stop(); \
}

#define DAA() [](CPU& cpu) { \
  word a = static_cast<word>(cpu.a); \
  if (!cpu.N()) { \
    if (cpu.H() || (a & 0xf) > 9) { \
      a += 0x06; \
    } \
    if (cpu.C() || (a > 0x9f)) { \
      a += 0x60; \
    } \
  } else { \
    if (cpu.H()) { \
      a = (a - 0x06) & 0xff; \
    } \
    if (cpu.C()) { \
      a -= 0x60; \
    } \
  } \
  cpu.unset_flags(Hf | Zf); \
  if ((a & 0x100) == 0x100) { \
    cpu.set_flags(Cf); \
  } \
  cpu.a = static_cast<byte>(a); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } \
}
