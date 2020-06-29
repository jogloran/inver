#pragma once

#include "types.h"

class Peripheral {
public:
  virtual byte read(word addr) = 0;
  virtual void write(word addr, byte value) = 0;

  virtual ~Peripheral() = default;
};
