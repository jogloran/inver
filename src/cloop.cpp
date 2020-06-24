#include <gflags/gflags.h>

#include "cloop.hpp"
#include "cpu6502.hpp"

DECLARE_bool(dis);

void LoopCondenser::observe(CPU6502& cpu) {
  if (cpu.pc == last_pc) return;

  history.emplace_back(cpu.pc);
  last_pc = cpu.pc;
  if (history.size() >= 128) {
    history.pop_front();
  }

  auto period = history_repeating(history);
  if (period) {
    repeating++;
    cpu.set_should_dump(repeating == 1);
  } else {
    if (FLAGS_dis && repeating > 0) {
      std::cout << "\t... [last " << std::dec << last_period << " ops repeated " << std::dec
                << repeating << " times]" << std::endl << std::endl;
    }

    repeating = 0;
    cpu.set_should_dump(true);
  }
  last_period = period;
}
