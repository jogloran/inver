#pragma once

#include <deque>

#include "util.h"

class CPU6502;

class LoopCondenser {
public:
  void observe(CPU6502& cpu);

private:
  std::deque<word> history;
  size_t repeating = 0;
  size_t last_period = 0;
  word last_pc = 0;
};
