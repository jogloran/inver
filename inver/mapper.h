#pragma once

#include <vector>

#include "types.h"

class Bus;

class Mapper {
public:
  virtual void map(const std::vector<char>&, byte prg_banks, byte chr_banks) = 0;
  virtual void connect(Bus* bus) = 0;

  virtual byte read(word addr) = 0;
  virtual void write(word addr, byte value) = 0;

  virtual byte chr_read(word addr) = 0;
  virtual void chr_write(word addr, byte value) = 0;

  virtual byte ppu_read(word addr) = 0;
  virtual void ppu_write(word addr, byte value) = 0;
};
