//
// Created by Daniel Tse on 9/7/20.
//

#include "cpu5a22.hpp"
#include "bus.hpp"
#include "ops_5a22.hpp"

void CPU5A22::tick() {
  if (cycles_left == 0) {
    byte opcode = bus->read(pc.addr);

    ++pc.addr;

    auto op = ops[opcode];
    cycle_count_t extra_cycles = op.f(*this);
    cycles_left = op.cyc + extra_cycles;
  }

  --cycles_left;
  ++ncycles;
}

void CPU5A22::reset() {

}
