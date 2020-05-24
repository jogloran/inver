#include "ppu_dmg.h"
#include "cpu.h"
#include "util.h"
#include "ppu_util.h"

#include <numeric>
#include <iterator>
#include <algorithm>
#include <sstream>

PPU::TileRow
GameBoyPPU::tilemap_index_to_tile(byte index, byte y_offset) {
  // There are 0x1000 bytes of tile data -- each entry is 0x10 bytes, so there are 0x100 entries
  // This takes each tile map index and retrieves
  // the corresponding line of the corresponding tile
  // 2 bytes per row, 16 bytes per tile
  // in each row, first byte is LSB of palette indices
  //              second byte is MSB
  if (bg_window_tile_data_offset == 0x8000) {
    // data ranges from 0x8000 (index 0) to 0x8fff (index 0xff)
    return decode(bg_window_tile_data_offset + index*16,
                  y_offset);
  } else {
    // add 0x800 to interpret the tile map index as a signed index starting in the middle
    // of the tile data range (0x8800-97FF)
    // data ranges from 0x8800 (index -127) to 0x9000 (index 0) to 0x97ff (index 128)
    return decode(bg_window_tile_data_offset + 0x800 + (static_cast<signed char>(index))*16,
                  y_offset);
  }
}

void
GameBoyPPU::rasterise_line() {
  // OAM is at 0xfe00 - 0xfea0 (40 sprites, 4 bytes each)
  byte* oam_ptr = &cpu.mmu.mem[0xfe00];
  OAM* oam = reinterpret_cast<OAM*>(oam_ptr);
  
  byte scx = cpu.mmu.mem[0xff43];
  byte scy = cpu.mmu.mem[0xff42];

  // For the sprite palettes, the lowest two bits should always map to 0
  byte obp0 = cpu.mmu.mem[0xff48] & 0xfc;
  byte obp1 = cpu.mmu.mem[0xff49] & 0xfc;
  
  // Background palette
  byte palette = cpu.mmu.mem[0xff47];

  if (bg_display) {
    auto row_touched = ((line + scy) % 256) / 8; // % 256 for scy wrap around
  
    // Create sequence of tiles to use (% 32 to wrap around)
    // Note that we actually take 21 tiles, because if
    // scx % 8 != 0, the raster may actually span part
    // of the first tile and part of the last
    auto starting_index = scx / 8;

    // Equivalent to:
    // 0<=i<=21, row_tiles[i] = cpu.mmu[bg_tilemap_offset + row_touched * 32 + ((starting_index + i) % 32)];
    auto* tilemap_ptr = &cpu.mmu[bg_tilemap_offset + row_touched * 32];
    rotate_tiles(starting_index, tilemap_ptr, row_tiles.begin());
    
    std::transform(row_tiles.begin(), row_tiles.end(), tile_data.begin(), [this, scx, scy](byte index) {
      return tilemap_index_to_tile(index, (line + scy) % 8);
    });
    
    flatten(tile_data, raster_row.begin());
    
    auto offset = static_cast<int>(scx % 8);

    std::copy_n(raster_row.begin() + offset, 160, palette_index_row.begin());
    
    std::transform(palette_index_row.begin(), palette_index_row.end(),
      raster.begin(), [palette](PaletteIndex idx) {
      return apply_palette(idx, palette);
    });
  }
  
  if (window_display) {
    byte wx = cpu.mmu.mem[0xff4b];
    byte wy = cpu.mmu.mem[0xff4a];
    
    if (line >= wy) {
      byte row_touched = (line - wy) / 8;

      auto* base = &cpu.mmu[window_tilemap_offset + row_touched * 32];
      std::transform(base, base + 20, tile_data.begin(), [this, wx, wy](byte index) {
        return tilemap_index_to_tile(index, (line - wy) % 8);
      });
      
      flatten(tile_data, raster_row.begin());

      std::transform(raster_row.begin(), raster_row.end(), raster_row.begin(), [palette](PaletteIndex idx) {
        return apply_palette(idx, palette);
      });
      
      // write to raster
      auto offset = static_cast<int>(wx - 7);
      if (offset < 160) {
        std::copy_n(raster_row.begin(), 160 - std::max(0, offset), raster.begin() + std::max(0, offset));
      }
    }
  }
  
  visible.clear();
  if (sprite_display) {
    auto sprite_height = sprite_mode == SpriteMode::S8x8 ? 8 : 16;
    
    for (size_t j = 0; j < 40; ++j) {
      OAM entry = oam[j];
      
      if (entry.x != 0 && entry.x < 168 && entry.y != 0 && entry.y < 160 &&
          line + 16 >= entry.y && line + 16 < entry.y + sprite_height) {
        
        auto tile_y = line - (entry.y - 16);
        if (entry.flags & (1 << 6)) { // y flip
          tile_y = sprite_height - tile_y - 1;
        }
        
        auto tile_index = entry.tile_index;
        if (sprite_mode == SpriteMode::S8x16) {
          if (tile_y < 8) {
            tile_index &= 0xfe;
          } else {
            tile_y -= 8;
            tile_index |= 0x01;
          }
        }
        
        // TODO: Need to work out how to modify tilemap_index_to_tile
        //       to hardcode 0x8000
        word tile_data_begin = 0x8000 + tile_index * 16;
        word tile_data_address = tile_data_begin + tile_y * 2;
        
        byte b1 = cpu.mmu[tile_data_address];
        byte b2 = cpu.mmu[tile_data_address + 1];

        // Map the sprite indices through the palette map
        auto decoded = unpack_bits(b1, b2);
        
        if (entry.flags & (1 << 5)) {
          std::reverse(decoded.begin(), decoded.end());
        }
        
        visible.emplace_back(RenderedSprite(entry, j, decoded));
      }
    }

    sprite_sort(visible);
    
    int sprites_rendered = 0;
    for (auto sprite : visible) {
      if (sprites_rendered++ == 10) {
        break;
      }
      
      auto bg_palette_index_ptr = palette_index_row.begin() + sprite.oam_.x - 8;
      auto raster_ptr = raster.begin() + sprite.oam_.x - 8;
      auto sprite_ptr = sprite.pixels_.begin();
      
      byte sprite_palette = (sprite.oam_.flags & (1 << 4)) ? obp1 : obp0;

      // If set to 0, sprite is always in front of bkgd and window
      // If set to 1, if background or window is colour 1, 2, 3, background or window wins
      //              else if background or window is color 0, sprite wins
      bool sprite_behind_bg = (sprite.oam_.flags & (1 << 7)) != 0;
      
      while (raster_ptr < raster.end() && sprite_ptr < sprite.pixels_.end()) {
        // hack to prevent invalid array access when a sprite starts before column 0
        if (raster_ptr >= raster.begin()) {
          // We need to examine the original palette byte, since the bg-to-OBJ
          // priority bit in LCDC needs to examine the original palette index
          // (and not the index after palette mapping)
          
          auto sprite_byte = *sprite_ptr; // Sprite palette index
          auto bg_palette_byte = *bg_palette_index_ptr; // Background palette index
          
          if (!(sprite_behind_bg && bg_palette_byte != 0)) {
            PPU::PaletteIndex idx = apply_palette(sprite_byte, sprite_palette);
            if (sprite_byte != 0) {
              *raster_ptr = idx;
            }
          }
        }
        
        ++raster_ptr; ++sprite_ptr; ++bg_palette_index_ptr;
      }
    }
  }
}

PPU::TileRow
GameBoyPPU::decode(word start_loc, byte start_y) {
  // start_y is from 0 to 7
  // we want row start_y of the tile
  // 2 bytes per row, 8 rows
  
  byte b1 = cpu.mmu.vram(start_loc + start_y*2, false);
  byte b2 = cpu.mmu.vram(start_loc + start_y*2 + 1, false);
  
  // b1/b2 is packed:
  // b1            b2
  // 00 01 10 11 | 10 11 00 10 LSB
  // 01 00 10 10 | 10 10 01 01 MSB
  // -------------------------
  // 02 02 30 31   30 31 02 12 <- we want to get this sequence
  
  return unpack_bits(b1, b2);
}

inline PPU::PaletteIndex apply_palette(PPU::PaletteIndex pidx, byte sprite_palette) {
  return (sprite_palette >> (pidx * 2)) & 3;
}

