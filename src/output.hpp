#pragma once

#include <array>
#include <chrono>

#include "types.h"

class PPU;
class Bus;

class Output {
public:
  virtual ~Output() = default;

  inline byte& at(size_t index) {
    return fb[index];
  }

  virtual void frame_rendered(std::chrono::milliseconds ms) = 0;

  virtual void set_paused(bool paused) {}

  static constexpr int BUF_WIDTH = 256;
  static constexpr int BUF_HEIGHT = 240;

  std::array<byte, BUF_WIDTH * BUF_HEIGHT> fb;
  PPU* ppu;
  Bus* bus;
};