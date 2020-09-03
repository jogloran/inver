#pragma once

#include <algorithm>

#include "types.h"

union colour_t {
  struct {
    byte r: 5;
    byte g: 5;
    byte b: 5;
    byte unused: 1;
  };
  word reg;

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(reg);
  }

  colour_t add(const colour_t& other, byte div) {
    return {
        .r = static_cast<byte>(std::min((this->r / div + other.r / div), 31)),
        .g = static_cast<byte>(std::min((this->g / div + other.g / div), 31)),
        .b = static_cast<byte>(std::min((this->b / div + other.b / div), 31))};
  }

  colour_t sub(const colour_t& other, byte div) {
    return {
        .r = static_cast<byte>(this->r / div <= other.r / div ? 0 : (this->r - other.r) / div),
        .g = static_cast<byte>(this->g / div <= other.g / div ? 0 : (this->g - other.g) / div),
        .b = static_cast<byte>(this->b / div <= other.b / div ? 0 : (this->b - other.b) / div)};
  }
};

namespace Colours {
  static constexpr colour_t BLACK {0, 0, 0};
}