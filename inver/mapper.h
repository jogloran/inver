#pragma once

#include <vector>

#include "header.hpp"
#include "types.h"

class Bus;

class Mapper {
public:
  enum class Mirroring {
    Unknown, V, H, Screen4, Screen1, ABBA, ABBB, ACBC, ABCC, ABBC, AAAA
  };

  virtual Mirroring get_mirroring() = 0;

  virtual void map(const std::vector<char>&, byte prg_banks, byte chr_banks, NESHeader* header) = 0;

  virtual byte read(word addr) = 0;
  virtual void write(word addr, byte value) = 0;

  virtual byte chr_read(word addr) = 0;
  virtual void chr_write(word addr, byte value) = 0;

  virtual void connect(Bus* b) {
    bus = b;
  }

private:
  Bus* bus;
};
