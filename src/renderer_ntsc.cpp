#include "renderer_ntsc.hpp"

void NTSCRenderer::render(const std::array<byte, 61440>& fb_data, size_t fb_width, byte* output,
                          size_t output_pitch, size_t output_height) {
  burst_phase ^= 1;

  nes_ntsc_blit(ntsc.get(), fb_data.data(), fb_width,
                burst_phase, fb_width, output_height,
                output, output_pitch);
}

std::pair<size_t, size_t> NTSCRenderer::get_output_size() {
  return {602, 240};
}
