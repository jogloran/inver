//
// Created by Daniel Tse on 9/7/20.
//

#include "cpu5a22.hpp"
#include "ops_5a22.hpp"
#include "bus_snes.hpp"

void CPU5A22::tick() {
  if (cycles_left == 0) {
    byte opcode = bus->read(pc.addr);

    std::printf("pc:%04x %02x a:%02x x:%02x y:%02x\n", pc.addr, opcode, a,x,y);

    ++pc.addr;

    auto op = ops_65c816[opcode];
    cycle_count_t extra_cycles = op.f(*this);
    cycles_left = op.cyc + extra_cycles;
  }

  --cycles_left;
  ++ncycles;
}

void CPU5A22::reset() {

}

void CPU5A22::connect(BusSNES* b) {
  bus = b;
}

void CPU5A22::write(word address, byte value) {
  bus->write(address, value);
}

byte CPU5A22::read(word address) {
  return bus->read(address);
}
