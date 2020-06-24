#pragma once

#include "peripheral.hpp"

class DevNull : public Peripheral {
public:
  byte read(word addr) override;

  void write(word addr, byte value) override;
};