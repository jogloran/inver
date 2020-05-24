#include "types.h"
#include "flags.h"
#include "cpu.h"

#define RLC8() RLC8_helper(b), \
RLC8_helper(c), \
RLC8_helper(d), \
RLC8_helper(e), \
RLC8_helper(h), \
RLC8_helper(l), \
RLC8_HL_helper(), \
RLC8_helper(a)

#define RRC8() RRC8_helper(b), \
RRC8_helper(c), \
RRC8_helper(d), \
RRC8_helper(e), \
RRC8_helper(h), \
RRC8_helper(l), \
RRC8_HL_helper(), \
RRC8_helper(a)

#define RL8() RL8_helper(b), \
RL8_helper(c), \
RL8_helper(d), \
RL8_helper(e), \
RL8_helper(h), \
RL8_helper(l), \
RL8_HL_helper(), \
RL8_helper(a)

#define RR8() RR8_helper(b), \
RR8_helper(c), \
RR8_helper(d), \
RR8_helper(e), \
RR8_helper(h), \
RR8_helper(l), \
RR8_HL_helper(), \
RR8_helper(a)

#define SLA8() SLA8_helper(b), \
SLA8_helper(c), \
SLA8_helper(d), \
SLA8_helper(e), \
SLA8_helper(h), \
SLA8_helper(l), \
SLA8_HL_helper(), \
SLA8_helper(a)

#define SRA8() SRA8_helper(b), \
SRA8_helper(c), \
SRA8_helper(d), \
SRA8_helper(e), \
SRA8_helper(h), \
SRA8_helper(l), \
SRA8_HL_helper(), \
SRA8_helper(a)

#define SWAP8() SWAP8_helper(b), \
SWAP8_helper(c), \
SWAP8_helper(d), \
SWAP8_helper(e), \
SWAP8_helper(h), \
SWAP8_helper(l), \
SWAP8_HL_helper(), \
SWAP8_helper(a)

#define SRL8() SRL8_helper(b), \
SRL8_helper(c), \
SRL8_helper(d), \
SRL8_helper(e), \
SRL8_helper(h), \
SRL8_helper(l), \
SRL8_HL_helper(), \
SRL8_helper(a)

#define BIT_GEN_HL(bit) [](CPU& cpu) { \
  cpu.unset_flags(Nf); \
  cpu.set_flags(Hf); \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  byte value = (cpu.mmu[loc] & (1 << bit)); \
  cpu.check_zero(value); \
}

#define BIT_GEN(bit, reg) [](CPU& cpu) { \
  cpu.unset_flags(Nf); \
  cpu.set_flags(Hf); \
  byte value = (cpu.reg & (1 << bit)); \
  cpu.check_zero(value); \
}

#define BIT_GEN8(bit) BIT_GEN(bit, b), \
BIT_GEN(bit, c), \
BIT_GEN(bit, d), \
BIT_GEN(bit, e), \
BIT_GEN(bit, h), \
BIT_GEN(bit, l), \
BIT_GEN_HL(bit), \
BIT_GEN(bit, a)

#define BIT_GEN64() BIT_GEN8(0), \
BIT_GEN8(1), \
BIT_GEN8(2), \
BIT_GEN8(3), \
BIT_GEN8(4), \
BIT_GEN8(5), \
BIT_GEN8(6), \
BIT_GEN8(7)

#define RES_GEN_HL(bit) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  cpu.mmu.set(loc, cpu.mmu[loc] & ~(1 << bit)); \
}

#define RES_GEN(bit, reg) [](CPU& cpu) { \
  cpu.reg &= ~(1 << bit); \
}

#define RES_GEN8(bit) RES_GEN(bit, b), \
RES_GEN(bit, c), \
RES_GEN(bit, d), \
RES_GEN(bit, e), \
RES_GEN(bit, h), \
RES_GEN(bit, l), \
RES_GEN_HL(bit), \
RES_GEN(bit, a)

#define RES_GEN64() RES_GEN8(0), \
RES_GEN8(1), \
RES_GEN8(2), \
RES_GEN8(3), \
RES_GEN8(4), \
RES_GEN8(5), \
RES_GEN8(6), \
RES_GEN8(7)

#define SET_GEN_HL(bit) [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  cpu.mmu.set(loc, cpu.mmu[loc] | (1 << bit)); \
}

#define SET_GEN(bit, reg) [](CPU& cpu) { \
  cpu.reg |= 1 << bit; \
}

#define SET_GEN8(bit) SET_GEN(bit, b), \
SET_GEN(bit, c), \
SET_GEN(bit, d), \
SET_GEN(bit, e), \
SET_GEN(bit, h), \
SET_GEN(bit, l), \
SET_GEN_HL(bit), \
SET_GEN(bit, a)

#define SET_GEN64() SET_GEN8(0), \
SET_GEN8(1), \
SET_GEN8(2), \
SET_GEN8(3), \
SET_GEN8(4), \
SET_GEN8(5), \
SET_GEN8(6), \
SET_GEN8(7)

#define RLC8_HL_helper() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  byte hibit = cpu.check_carry_if_hibit_set(cpu.mmu[loc]); \
  cpu.mmu.set(loc, (cpu.mmu[loc] << 1) | hibit); \
  cpu.check_zero(cpu.mmu[loc]); \
  cpu.unset_flags(Nf | Hf); \
}

#define RLC8_helper(reg) [](CPU& cpu) { \
  byte hibit = cpu.check_carry_if_hibit_set(cpu.reg); \
  cpu.reg = (cpu.reg << 1) | hibit; \
  cpu.check_zero(cpu.reg); \
  cpu.unset_flags(Nf | Hf); \
}

#define RRC8_HL_helper() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  byte lobit = cpu.check_carry_if_lobit_set(cpu.mmu[loc]); \
  cpu.mmu.set(loc, (cpu.mmu[loc] >> 1) | (lobit << 7)); \
  cpu.check_zero(cpu.mmu[loc]); \
  cpu.unset_flags(Nf | Hf); \
}

#define RRC8_helper(reg) [](CPU& cpu) { \
  byte lobit = cpu.check_carry_if_lobit_set(cpu.reg); \
  cpu.reg = (cpu.reg >> 1) | (lobit << 7); \
  cpu.check_zero(cpu.reg); \
  cpu.unset_flags(Nf | Hf); \
}

#define SWAP8_HL_helper() [](CPU& cpu) { \
  cpu.unset_flags(Nf | Hf | Cf); \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  byte val = cpu.mmu[loc]; \
  cpu.mmu.set(loc, (val << 4) | (val >> 4)); \
  cpu.check_zero(cpu.mmu[loc]); \
}

#define SWAP8_helper(reg) [](CPU& cpu) { \
  cpu.unset_flags(Nf | Hf | Cf); \
  cpu.reg = (cpu.reg << 4) | (cpu.reg >> 4); \
  cpu.check_zero(cpu.reg); \
}

#define RL8_HL_helper() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  byte carry = cpu.C(); \
  cpu.check_carry_if_hibit_set(cpu.mmu[loc]); \
  cpu.mmu.set(loc, (cpu.mmu[loc] << 1) | carry); \
  cpu.check_zero(cpu.mmu[loc]); \
  cpu.unset_flags(Nf | Hf); \
}

#define RL8_helper(reg) [](CPU& cpu) { \
  byte carry = cpu.C(); \
  cpu.check_carry_if_hibit_set(cpu.reg); \
  cpu.reg = (cpu.reg << 1) | carry; \
  cpu.check_zero(cpu.reg); \
  cpu.unset_flags(Nf | Hf); \
}

#define RR8_HL_helper() [](CPU& cpu) { \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  byte carry = cpu.C(); \
  cpu.check_carry_if_lobit_set(cpu.mmu[loc]); \
  cpu.mmu.set(loc, (cpu.mmu[loc] >> 1) | (carry << 7)); \
  cpu.check_zero(cpu.mmu[loc]); \
  cpu.unset_flags(Nf | Hf); \
}

#define RR8_helper(reg) [](CPU& cpu) { \
  byte carry = cpu.C(); \
  cpu.check_carry_if_lobit_set(cpu.reg); \
  cpu.reg >>= 1; \
  cpu.reg |= carry << 7; \
  cpu.check_zero(cpu.reg); \
  cpu.unset_flags(Nf | Hf); \
}

#define SLA8_HL_helper() [](CPU& cpu) { \
  cpu.unset_flags(Nf | Hf); \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  cpu.check_carry_if_hibit_set(cpu.mmu[loc]); \
  cpu.mmu.set(loc, cpu.mmu[loc] << 1); \
  cpu.check_zero(cpu.mmu[loc]); \
}

#define SLA8_helper(reg) [](CPU& cpu) { \
  cpu.unset_flags(Nf | Hf); \
  cpu.check_carry_if_hibit_set(cpu.reg); \
  cpu.reg <<= 1; \
  cpu.check_zero(cpu.reg); \
}

#define SRA8_HL_helper() [](CPU& cpu) { \
  cpu.unset_flags(Nf | Hf); \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  byte hibit = cpu.mmu[loc] & 0x80; \
  cpu.check_carry_if_lobit_set(cpu.mmu[loc]); \
  cpu.mmu.set(loc, (cpu.mmu[loc] >> 1) | hibit); \
  cpu.check_zero(cpu.mmu[loc]); \
}

#define SRA8_helper(reg) [](CPU& cpu) { \
  cpu.unset_flags(Nf | Hf); \
  byte hibit = cpu.reg & 0x80; \
  cpu.check_carry_if_lobit_set(cpu.reg); \
  cpu.reg >>= 1; \
  cpu.reg |= hibit; \
  cpu.check_zero(cpu.reg); \
}

#define SRL8_HL_helper() [](CPU& cpu) { \
  cpu.unset_flags(Nf | Hf); \
  word loc = cpu.get_word(cpu.h, cpu.l); \
  cpu.check_carry_if_lobit_set(cpu.mmu[loc]); \
  cpu.mmu.set(loc, cpu.mmu[loc] >> 1); \
  cpu.check_zero(cpu.mmu[loc]); \
}

#define SRL8_helper(reg) [](CPU& cpu) { \
  cpu.unset_flags(Nf | Hf); \
  cpu.check_carry_if_lobit_set(cpu.reg); \
  cpu.reg >>= 1; \
  cpu.check_zero(cpu.reg); \
}
