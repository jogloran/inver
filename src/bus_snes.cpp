//
// Created by Daniel Tse on 10/7/20.
//

#include "bus_snes.hpp"

void BusSNES::tick() {
  cpu.tick();
}

void BusSNES::reset() {
  cpu.reset();
}

dual BusSNES::read(word address) {
  return { ram[address] };
}

void BusSNES::write(word address, byte value) {
  ram[address] = value;
}
