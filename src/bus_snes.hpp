#pragma once

#include <array>
#include "types.h"
#include "cpu5a22.hpp"

class BusSNES {
public:
  BusSNES() {
    cpu.connect(this);
  }

  void tick();

  void reset();

  byte read(word address);

  void write(word address, byte value);

  std::array<byte, 0x20000> ram {};

  CPU5A22 cpu;
};