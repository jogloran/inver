#pragma once

#include <algorithm>
#include <array>
#include <iterator>
#include <random>
#include <vector>

#include "types.h"

class Mapper {
public:
  Mapper() {
    auto gen_rand_byte = [this]() { return memory_filler(generator); };
    std::generate(sram1.begin(), sram1.end(), gen_rand_byte);
    std::generate(sram2.begin(), sram2.end(), gen_rand_byte);
  }

  virtual byte read(dword address) = 0;

  virtual void write(dword address, byte value) = 0;

  virtual void reset() = 0;

  virtual void map(std::vector<byte>&& data) = 0;

  byte* flash(byte* ptr, size_t len, std::vector<byte>& out) {
    std::copy(ptr, ptr + len, std::back_inserter(out));
    return ptr + len;
  }

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(rom, sram1, sram2);
  }

protected:
  std::vector<byte> rom;
  std::array<byte, 0x8000 * 0xd> sram1 {};
  std::array<byte, 0x8000 * 0x10> sram2 {};

private:
  std::default_random_engine generator;
  std::uniform_int_distribution<byte> memory_filler;
};