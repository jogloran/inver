#include "ppu_cgb.h"
#include "cpu.h"
#include "util.h"
#include "ppu_util.h"

#include <numeric>
#include <iterator>
#include <algorithm>
#include <sstream>

inline byte encode_palette(byte palette_number, bool sprite_palette, byte palette_index, bool bg_priority=false) {
  // palette number can be 0-7
  // bg/win palette numbers are from 0*8+0*2  = 0   to 7*8+4*2  = 64
  // with bg priority flag      from            1   to            65
  // sprite palette numbers are from 16*8+0*2 = 128 to 23*8+4*2 = 192
  return ((sprite_palette ? 16 : 0) + palette_number) * 8 + palette_index * 2 + bg_priority;
}

inline bool bg_master_priority(byte encoded_palette) {
  return encoded_palette & 0b1;
}

PPU::TileRow
ColorGameBoyPPU::tilemap_index_to_tile(byte index, byte y_offset, bool flip_horizontal, bool use_alt_bank, bool force_8000_offset) {
  // There are 0x1000 bytes of tile data -- each entry is 0x10 bytes, so there are 0x100 entries
  word tile_data_offset = force_8000_offset ? 0x8000 : bg_window_tile_data_offset;
  if (tile_data_offset == 0x8000) {
    // data ranges from 0x8000 (index 0) to 0x8fff (index 0xff)
    return decode(tile_data_offset + index*16,
                  y_offset, flip_horizontal, use_alt_bank);
  } else {
    // add 0x800 to interpret the tile map index as a signed index starting in the middle
    // of the tile data range (0x8800-97FF)
    // data ranges from 0x8800 (index -127) to 0x9000 (index 0) to 0x97ff (index 128)
    return decode(tile_data_offset + 0x800 + (static_cast<signed char>(index))*16,
                  y_offset, flip_horizontal, use_alt_bank);
  }
}

void
ColorGameBoyPPU::rasterise_line() {
  // OAM is at 0xfe00 - 0xfea0 (40 sprites, 4 bytes each)
  byte* oam_ptr = &cpu.mmu.mem[0xfe00];
  OAM* oam = reinterpret_cast<OAM*>(oam_ptr);
  
  byte scx = cpu.mmu.mem[0xff43];
  byte scy = cpu.mmu.mem[0xff42];

  if (bg_display) {
    auto row_touched = ((line + scy) % 256) / 8; // % 256 for scy wrap around
  
    auto starting_index = scx / 8;
    
    auto* cgb_attrs_base = &cpu.mmu.vram(bg_tilemap_offset + row_touched * 32, true);
    rotate_tiles(starting_index, cgb_attrs_base, cgb_attr_tiles.begin());
    
    auto* tilemap_ptr = &cpu.mmu.vram(bg_tilemap_offset + row_touched * 32, false);
    rotate_tiles(starting_index, tilemap_ptr, row_tiles.begin());
    
    auto row_tile_ptr = row_tiles.begin();
    auto cgb_attrs_ptr = cgb_attr_tiles.begin();
    auto tile_data_ptr = tile_data.begin();
    for (; row_tile_ptr != row_tiles.end(); ++row_tile_ptr, ++cgb_attrs_ptr) {
      byte cgb_attr = *cgb_attrs_ptr;
      
      bool vram_bank = cgb_attr & (1 << 3);
      bool flip_horizontal = cgb_attr & (1 << 5);
      bool flip_vertical = cgb_attr & (1 << 6);
      
      byte y_offset = (line + scy) % 8;
      if (flip_vertical) {
        y_offset = 7 - y_offset;
      }
      
      byte index = *row_tile_ptr;
      *tile_data_ptr++ = tilemap_index_to_tile(index, y_offset, flip_horizontal, vram_bank);
    }
    flatten(tile_data, raster_row.begin());
    
    auto offset = static_cast<int>(scx % 8);

    std::copy_n(raster_row.begin() + offset, 160, palette_index_row.begin());
    
    // Why (i + offset) / 8?
    // Consider scx = 5 so that there's a partial first and last tile
    // partial tile                   partial tile
    //       |  |--(19 whole tiles)-|   |
    // xxxxxOOO OOOOOOOO ... OOOOOOOO OOOOOxxx
    // raster index:
    //      012 34567890 ...
    // + offset
    //      567 89012345 ...
    // divided by 8
    //    0        1     ...
    // We need to add the offset (5) back to raster index to work out
    // which CGB attr tile it corresponds to
    for (int i = 0; i < palette_index_row.size(); ++i) {
      byte cgb_attr = cgb_attr_tiles[(i + offset) / 8];
      bool bg_master_priority = cgb_attr & (1 << 7);
      byte palette_index = cgb_attr & 0b111;
      
      raster[i] = encode_palette(palette_index, false, palette_index_row[i], bg_master_priority);
    }
  }
  
  if (window_display) {
    byte wx = cpu.mmu.mem[0xff4b];
    byte wy = cpu.mmu.mem[0xff4a];
    
    if (line >= wy) {
      byte row_touched = (line - wy) / 8;
      
      auto* cgb_attrs = &cpu.mmu.vram(window_tilemap_offset + row_touched * 32, true);
      auto* base = &cpu.mmu.vram(window_tilemap_offset + row_touched * 32, false);
      std::transform(base, base + 20, tile_data.begin(), [this, wx, wy](byte index) {
        return tilemap_index_to_tile(index, (line - wy) % 8);
      });
      
      flatten(tile_data, raster_row.begin());
      
      auto offset = static_cast<int>(wx - 7);
      std::copy_n(raster_row.begin(), 160 - std::max(0, offset), palette_index_row.begin() + offset);
      
      for (int i = 0; i < raster_row.size(); ++i) {
        raster_row[i] = encode_palette(cgb_attrs[i / 8] & 0b111, false, raster_row[i]);
      }
      
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
        
        byte cgb_palette = entry.flags & 0x3;
        bool cgb_vram_bank = entry.flags & (1 << 3);
        bool horizontal_flip = entry.flags & (1 << 5);
        
        auto tile_index = entry.tile_index;
        if (sprite_mode == SpriteMode::S8x16) {
          if (tile_y < 8) {
            tile_index &= 0xfe;
          } else {
            tile_y -= 8;
            tile_index |= 0x01;
          }
        }
        
        auto decoded = tilemap_index_to_tile(tile_index, tile_y, horizontal_flip, cgb_vram_bank, true);
        
        visible.emplace_back(RenderedSprite(entry, j, decoded, cgb_palette, cgb_vram_bank));
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
      
      bool sprite_behind_bg = (sprite.oam_.flags & (1 << 7)) != 0;
      
      while (raster_ptr < raster.end() && sprite_ptr < sprite.pixels_.end()) {
        // hack to prevent invalid array access when a sprite starts before column 0
        if (raster_ptr >= raster.begin()) {
          auto sprite_byte = *sprite_ptr; // Sprite palette index
          auto bg_palette_byte = *bg_palette_index_ptr; // Background palette index
          
          // Conditions for sprite to overlap background:
          // - bg does not have master priority, AND
          // - it is not the case that:
          //     - sprite should be behind BG, AND
          //     - BG is not transparent
          if (!bg_master_priority(*raster_ptr) && !(sprite_behind_bg && bg_palette_byte != 0)) {
            // don't draw sprite (i.e. do nothing and let background shine through) if sprite_behind_bg and bg_palette_byte != 0
            // i.e. draw sprite if not (sprite_behind_bg and bg_palette_byte != 0), i.e. if !sprite_behind_bg or bg_palette_byte == 0
            
            // The problem is that we are drawing the sprite for all 8x8, not just where bg_palette_byte == 0
            if (sprite_byte % 8 != 0) {
              // Use 16 + palette no to indicate OBPx instead of BGPx
              *raster_ptr = encode_palette(sprite.cgb_palette_, true, sprite_byte);
            }
          }
        }
        
        ++raster_ptr; ++sprite_ptr; ++bg_palette_index_ptr;
      }
    }
  }
}

void flip_bits(PPU::TileRow& tile) {
  std::reverse(tile.begin(), tile.end());
}

PPU::TileRow
ColorGameBoyPPU::decode(word start_loc, byte start_y, bool flip_horizontal, bool use_alt_bank) {
  // start_y is from 0 to 7
  // we want row start_y of the tile
  // 2 bytes per row, 8 rows
  
  word address = start_loc + start_y*2;
  byte b1 = cpu.mmu.vram(address, use_alt_bank);
  byte b2 = cpu.mmu.vram(address + 1, use_alt_bank);
  
  // b1/b2 is packed:
  // b1            b2
  // 00 01 10 11 | 10 11 00 10 LSB
  // 01 00 10 10 | 10 10 01 01 MSB
  // -------------------------
  // 02 02 30 31   30 31 02 12 <- we want to get this sequence
  
  auto result = unpack_bits(b1, b2);
  if (flip_horizontal) {
    flip_bits(result);
  }
  return result;
}
