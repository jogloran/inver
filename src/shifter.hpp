#pragma once

#include "types.h"

class Shifter {
public:
  void shift() {
    v[0] <<= 1;
    v[1] <<= 1;
  }

  void load(byte lsb, byte msb) {
    v[0] = (v[0] & 0xff00) | lsb;
    v[1] = (v[1] & 0xff00) | msb;
  }

  byte operator()(word mask) {
    // apply fine_x:
    // fine_x = 0 => take bit 7 of shifters
    // fine_x = 1 => take bit 6 ...
    //   ...  = 7 => take bit 0 ...
    // select one of the top 8 bits
    return !!(v[0] & mask) | (!!(v[1] & mask) << 1);
  }

  template <typename Ar>
  void serialize(Ar& ar) {
    ar(v);
  }

private:
  word v[2];
};