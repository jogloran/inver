#pragma once

#include <cstdio>
#include <string>

#include "cpu5a22.hpp"
#include "ops_5a22.hpp"

enum Arg {
  IndX, Zp, Imm, Abs, YInd, ZpX,
  AbsY, AbsX, Acc,
  Implied, Invalid, Rel, AbsDword, AbsDwordX,
  ZpFar, StkImm, StkImmIndY, RelFar, ZpFarY,
  AbsXInd, BytePair, ImmDword, AbsFar, Ind,
};

struct Op {
  byte opcode;
  std::string mnemonic;
  Arg addr_mode;
};

std::array<Op, 256> build_ops();
extern std::array<Op, 256> op_meta;

void dump_op_table(std::ostream& o);

std::string instruction_at_pc(CPU5A22& cpu);
