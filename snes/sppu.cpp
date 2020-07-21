#include "sppu.hpp"
#include "bus_snes.hpp"

// table of (mode, layer) -> bpp?

void SPPU::render_row() {
  // get bg mode
  byte mode = bgmode.mode;

  // assume we're looking at 4bpp layer 0
  // get base address for this layer
  dword tilemap_base_addr = bg_base_size[2].base_addr * 0x400;

  // 1024 words after this addr correspond to the current tilemap
  byte cur_row = line / 8;
  byte tile_row = line % 8;

  dword chr_base_addr = bg_char_data_addr[1].bg1_tile_base_addr << 12;

  std::array<word, 32> tile_ids;
  dword tilemap_offs_addr = tilemap_base_addr + cur_row * 32;

  std::transform(&vram[tilemap_offs_addr], &vram[tilemap_offs_addr + 32],
                 tile_ids.begin(),
                 [](dual w) {
                   SPPU::bg_map_tile_t* t = (SPPU::bg_map_tile_t*) &w;
                   return t->char_no;
                 });

  byte bpp = 2; // 2bpp means one pixel is encoded in one word
  bpp /= 2;

  // coming into this, we get 32 tile ids. for each tile id, we want to decode 8 palette values:
  int col = 0;
  std::for_each(tile_ids.begin(), tile_ids.end(),
                [&](auto tile_id) {
                  auto* fb_ptr = screen->fb[0].data() + line * 256 + col * 8;

                  // get tile chr data
                  word tile_chr_base = chr_base_addr + (8 * bpp) * tile_id + tile_row;
                  std::printf("> %06x (base %06x tile_id %04x tile_row %02x)\n", tile_chr_base,
                      chr_base_addr, tile_id, tile_row);

                  // read bpp bytes (bpp/2 words)
                  std::vector<word> data { vram[tile_chr_base].w };

                  // produce 8 byte values (palette indices)
                  std::vector<word> pal_data;
                  for (int i = 0; i < 8*bpp; i += bpp) {
                    // take lsb and msb and merge them together
                    byte lsb = data[0] & 0xff;
                    byte msb = data[0] >> 8;
                    pal_data.emplace_back( !!(lsb & (1 << (7 - i))) + 2 * !!(msb & (1 << (7 - i))) );
                  }

                  // decode bpp subsequent bytes into a single value

                  for (int i = 0; i < 8; ++i) {
                    // fill in 8 words from fb_ptr with colour values for one tile (tile_id)
                    fb_ptr->r = pal_data[i];
                    fb_ptr->g = pal_data[i];
                    fb_ptr->b = pal_data[i];
//                    std::printf("wrote pal_data %x into fb_ptr %02x %02x %02x\n", pal_data[i], fb_ptr->r, fb_ptr->g ,fb_ptr->b);

                    ++fb_ptr;
                  }

                  ++col;
                });

  // determine which tiles are in viewport
  // 0...32

  // for each layer:
  // lookup bpp for mode and layer
  // write row of 256 to screen->fb[0]
}

void SPPU::tick(byte master_cycles) {
  ncycles += master_cycles;

  switch (state) {
    case State::VISIBLE:
      x = ncycles / 4;
      if (ncycles >= 4 * 256) {
        ncycles -= 4 * 256;
        state = State::HBLANK;

        render_row();
//        log("%-3ld x=%d line=%-3d vis -> hbl\n", x, ncycles, line);
      }
      break;
    case State::HBLANK:
      if (ncycles >= 84 * 4) {
        ncycles -= 84 * 4;
        ++line;
        x = 0;

        if (line <= 0xe0) {
          state = State::VISIBLE;
//          log("%-3ld x=%d line=%-3d hbl -> vis\n", x, ncycles, line);
        } else if (line == 0xe1) {
          state = State::VBLANK;
//          log("%-3ld x=%d line=%-3d hbl -> vbl\n", x, ncycles, line);
          bus->vblank_nmi();
        }
      }
      break;
    case State::VBLANK:
      if (ncycles >= 340 * 4) {
        ncycles -= 340 * 4;
        ++line;
        x = 0;
        // TODO: status flags need to distinguish hblank even during
        // vblank

        if (line >= 0x106) {
          bus->vblank_end();
          screen->blit();
          state = State::VISIBLE;
          log("%-3ld x=%d line=%-3d vbl -> vis\n", x, ncycles, line);
          line = 0;
        }
      }
      break;
  }

  auto hv_irq = bus->nmi.hv_irq;
  bool fire_irq = false;
  if (hv_irq == 1 && x == htime.v) {
    fire_irq = true;
  } else if (hv_irq == 2 && x == 0 && line == vtime.v) {
    fire_irq = true;
  } else if (hv_irq == 3 && x == htime.v && line == vtime.v) {
    fire_irq = true;
  }
  if (fire_irq) {
    bus->raise_timeup();
  }
}