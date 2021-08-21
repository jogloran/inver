#pragma once
#include <cstdint>

using sbyte = int8_t;
using sword = int16_t;
using sdword = int32_t;
using byte = uint8_t;
using word = uint16_t;
using dword = uint32_t;

using cycle_count_t = int;

/**
 * Represents the result of the hardware multiplication operation.
 */
union s24_t {
  struct {
    byte l : 8;
    byte m : 8;
    byte h : 8;
    byte unused : 8;
  };
  sdword w;
};

union dual {
  operator word() {
    return w;
  }

  dual operator++() {
    ++w;
    return *this;
  }

  dual operator--() {
    --w;
    return *this;
  }

  dual operator++(int) {
    dual copy(*this);
    operator++();
    return copy;
  }

  dual operator--(int) {
    dual copy(*this);
    operator--();
    return copy;
  }

  struct {
    byte l: 8;
    byte h: 8;
  };
  // Aliases for M7
  struct {
    byte m7_tile_id: 8;
    // in direct colour mode, a value to be interpreted as a colour.
    // otherwise, a palette index
    byte m7_chr: 8;
  };
  word w;


  template<typename Ar>
  void serialize(Ar& ar) {
    ar(w);
  }
};