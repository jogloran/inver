#pragma once

#include "screen.h"
#include <iostream>
#include <sstream>

extern bool ASOBITOMO_DEBUG;

class Console : public Screen {
public:
  static const char* d[];
  
  void blit() {
    if (ASOBITOMO_DEBUG) {
      std::stringstream s;
      s << "\033[2J\033[1;1H";
      
      for (int i = 0; i < BUF_HEIGHT; ++i) {
        for (int j = 0; j < BUF_WIDTH; ++j) {
          s << d[fb[BUF_WIDTH*i + j]] << d[fb[BUF_WIDTH*i + j]];
        }
        s << std::endl;
      }
      
      std::cout << s.str() << std::endl;
    }
  }
};
