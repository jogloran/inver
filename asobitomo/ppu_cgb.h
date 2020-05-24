#pragma once

#include "types.h"
#include "screen.h"
#include "console_screen.h"
#include "gl_screen.h"
#include "tile_debugger.h"
#include "tile_map.h"
#include "ppu_base.h"

#include <memory>
#include <array>
#include <gflags/gflags.h>

DECLARE_bool(tm);
DECLARE_bool(td);

class ColorGameBoyPPU : public PPU {
public:
  struct RenderedSprite {
    RenderedSprite(const OAM& oam, byte oam_index, const TileRow& pixels, byte cgb_palette, bool cgb_ram_bank): oam_(oam), oam_index_(oam_index), pixels_(pixels),
    cgb_palette_(cgb_palette), cgb_ram_bank_(cgb_ram_bank) {}
    OAM oam_;
    byte oam_index_;
    TileRow pixels_;
    byte cgb_palette_;
    bool cgb_ram_bank_;
  };
  
  ColorGameBoyPPU(CPU& cpu):
    PPU(cpu, std::make_unique<TD>(*this), std::make_unique<TM>(*this)) {
    visible.reserve(40);
  }
  
  void rasterise_line();
  
  TileRow decode(word start_loc, byte start_y=0 /* 0 to 7 */, bool flip_horizontal=false, bool use_alt_bank=false);
  
  TileRow tilemap_index_to_tile(byte index, byte y_offset, bool flip_horizontal=false, bool use_alt_bank=false, bool force_8000_offset=false);
  TileRow tilemap_index_to_tile_debug(byte index, byte y_offset) {
    return tilemap_index_to_tile(index, y_offset);
  }
  
public:
  std::vector<RenderedSprite> visible;
  
  // Caches
  std::array<PPU::PaletteIndex, 160> palette_index_row;
  std::array<word, 21> row_tiles;
  std::array<byte, 21> cgb_attr_tiles;
  
  std::array<PaletteIndex, 168> raster_row;
  std::array<TileRow, 21> tile_data;
};
