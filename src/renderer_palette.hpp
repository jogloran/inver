#pragma once

#include <array>
#include "types.h"
#include "renderer.hpp"

class PaletteRenderer : public Renderer {
public:
  void render(const std::array<byte, 61440>& fb_data, size_t fb_width, byte* output, size_t output_pitch,
              size_t output_height) override;

  std::pair<size_t, size_t> get_output_size() override {
    return {256, 240};
  }

  PixelFormat get_pixel_format() override {
    return PixelFormat::ARGB8888;
  }
};
