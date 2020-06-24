//
// Created by Daniel Tse on 24/6/20.
//

#include "dev_null.hpp"

byte DevNull::read(word addr) {
  return 0;
}

void DevNull::write(word addr, byte value) {

}
