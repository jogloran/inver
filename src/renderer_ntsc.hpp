#pragma once

#include <array>
#include "nes_ntsc.h"
#include "types.h"
#include "renderer.hpp"

class NTSCRenderer : public Renderer {
public:
  NTSCRenderer() {
    ntsc = std::make_unique<nes_ntsc_t>();
    nes_ntsc_setup_t setup = nes_ntsc_composite;
    nes_ntsc_init(ntsc.get(), &setup);
  }

  std::pair<size_t, size_t> get_output_size() override;

  PixelFormat get_pixel_format() override {
    return PixelFormat::RGB565;
  }

  void render(const std::array<byte, 61440>& fb_data, size_t fb_width, byte* output,
              size_t output_pitch, size_t output_height) override;

private:
  int burst_phase = 0;
  std::unique_ptr<nes_ntsc_t> ntsc;
};

