#pragma once

#include <chrono>
#include <iomanip>

#include "output.hpp"

using namespace std::chrono_literals;

class NullOutput : public Output {
public:
  void frame_rendered(std::chrono::milliseconds ms) override {
    auto now = std::chrono::high_resolution_clock::now();
    auto datum = 1000ms / (now - last_frame_rendered);

    last_frame_rendered = now;

    fps = ALPHA * datum + (1. - ALPHA) * fps;
    std::cout << "\33[2K\r" << std::fixed << std::showpoint << std::setprecision(2) << std::setw(7) << fps << " fps" << std::flush;
  }

private:
  std::chrono::high_resolution_clock::time_point last_frame_rendered;
  double fps;

  static constexpr double ALPHA = 0.3;
};