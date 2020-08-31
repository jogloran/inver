#pragma once

#include "mapper.hpp"

class HiROM : public Mapper {
public:
  HiROM() {

  }

  byte read(dword address) override {
    return 0;
  }
  void write(dword address, byte value) override {
  }
  void reset() override {
  }
};