#pragma once

#include <vector>
#include <array>
#include <algorithm>
#include <functional>
#include <iostream>

#include "types.h"

class Screen {
public:
  Screen(): fb(), should_draw(true) {}

  template <typename T>
  void set_row(int row, T begin, T end) {
#ifndef NDEBUG
    if (row < 0 || row >= BUF_HEIGHT) {
      throw std::range_error("invalid row");
    }
    if (std::distance(begin, end) != BUF_WIDTH) {
      throw std::range_error("invalid range");
    }
#endif
    std::copy_n(begin, BUF_WIDTH, fb.begin() + row * BUF_WIDTH);
  }
  virtual void blit() = 0;
  
  using exit_handler = std::function<void()>;
  void add_exit_handler(exit_handler callable) {
    exit_handlers.emplace_back(callable);
  }
  
  void off() { should_draw = false; }
  void on() { should_draw = true; }
  
  static constexpr int BUF_WIDTH = 160;
  static constexpr int BUF_HEIGHT = 144;
  
protected:
  void notify_exiting() {
    for (exit_handler ex : exit_handlers) {
      ex();
    }
    exit(0);
  }
  
  bool should_draw;
  std::array<byte, BUF_WIDTH * BUF_HEIGHT> fb;
  
private:
  std::vector<exit_handler> exit_handlers;
};
