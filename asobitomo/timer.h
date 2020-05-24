#pragma once

#include "types.h"

class CPU;

class Timer {
public:
  Timer(CPU& cpu): cpu(cpu), divider(0x0), divider_hi(0x0),
    counter(0x0), counter_cycles(0x0),
    modulo(0xff),
    tac_(0), enabled(false), speed(1024) {}
  
  void inc_tima();
  
  void step(long delta);
  void fire_timer_interrupt();
  
  byte& div();
  void reset_div();
  
  byte& tac();
  void set_tac(byte value);
  
  CPU& cpu;
  word divider;
  byte divider_hi;
  
  byte counter;
  long counter_cycles;
  
  byte modulo;
  
  byte tac_;
  bool enabled;
  int speed;
  
  constexpr static int CYCLES_PER_SECOND = 4'194'304;
};
