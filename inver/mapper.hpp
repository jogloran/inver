#pragma once

#include <vector>

#include "types.h"

class Bus;
class NESHeader;

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

  virtual bool irq_requested() { return false; }
  virtual void irq_handled() {}

  virtual void signal_scanline() {}

  virtual void reset() = 0;

  virtual void connect(Bus* b) {
    bus = b;
  }

protected:
  Bus* bus;
};
