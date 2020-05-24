#pragma once

#include "types.h"
#include <string>

class MBCBase {
public:
  virtual byte* get(word loc) = 0;
  virtual bool set(word loc, byte value) = 0;
  
  virtual void save(std::string path) {}
  virtual void load(std::string path) {}
  
  virtual int bank_no() = 0;
  virtual int ram_bank_no() { return -1; }
};
