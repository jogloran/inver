#pragma once
#include <cstdint>

using sbyte = int8_t;
using sword = int16_t;
using sdword = int32_t;
using byte = uint8_t;
using word = uint16_t;
using dword = uint32_t;

using cycle_count_t = int;

union s24_t {
  struct {
    sbyte l : 8;
    sbyte m : 8;
    sbyte h : 8;
    sbyte unused : 8;
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
  word w;


  template<typename Ar>
  void serialize(Ar& ar) {
    ar(w);
  }
};