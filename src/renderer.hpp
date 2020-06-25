#pragma once

#include <array>
#include "types.h"

class Renderer {
public:
  enum class PixelFormat {
    RGB565, ARGB8888,
  };
  virtual ~Renderer() = default;

  virtual void init() {}

  virtual PixelFormat get_pixel_format() = 0;

  virtual std::pair<size_t, size_t> get_output_size() = 0;

  virtual void render(const std::array<byte, 61440>& fb_data, size_t fb_width, byte* output,
                      size_t output_pitch, size_t output_height) = 0;
};

