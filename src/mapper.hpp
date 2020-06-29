#pragma once

#include <vector>

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>

#include "types.h"

class Bus;
struct NESHeader;

class Mapper {
public:
  virtual ~Mapper() = default;

  enum class Mirroring {
    Unknown, V, H, Screen4, Screen1, ABBA, ABBB, ACBC, ABCC, ABBC, AAAA, BBBB
  };

  virtual Mirroring get_mirroring() = 0;

  virtual void map(const std::vector<char>&, byte prg_banks, byte chr_banks, const NESHeader* header) = 0;
  virtual void map_ram(const std::vector<char>&, size_t len) {}
  virtual byte* get_ram() { return nullptr; }

  virtual byte read(word addr) = 0;
  virtual void write(word addr, byte value) = 0;

  virtual byte chr_read(word addr) = 0;
  virtual void chr_write(word addr, byte value) = 0;

  virtual bool irq_requested() { return false; }

  virtual void signal_scanline() {}

  virtual void dump_mapper() {}

  virtual void reset() = 0;

  virtual void connect(Bus* b) {
    bus = b;
  }

protected:
  Bus* bus;

  byte* flash(byte* ptr, size_t len, std::vector<byte>& out) {
    std::copy(ptr, ptr + len, std::back_inserter(out));
    return ptr + len;
  }
};
