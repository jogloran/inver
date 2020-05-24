#pragma once

#include <cstdio>
#include <string>

#include "cpu6502.hpp"
#include "ops.hpp"

enum Arg {
  IndX, Zp, Imm, Abs, YInd, ZpX,
  AbsY, AbsX, Acc,
  Implied, Invalid, Rel
};

struct Op {
  byte opcode;
  std::string mnemonic;
  Arg addr_mode;
};

std::array<Op, 256> build_ops();
extern std::array<Op, 256> op_meta;

std::string instruction_at_pc(CPU6502& cpu);
