#include "sppu.hpp"
#include "bus_snes.hpp"
#include "ppu_utils.hpp"

// table of (mode, layer) -> bpp?

std::array<byte, 3> bpps = { {4, 4, 2} };

void SPPU::dump_bg() {
  dword tilemap_base_addr = bg_base_size[2].base_addr * 0x400;
  for (int cur_row = 0; cur_row < 32; ++cur_row) {
    dword tilemap_offs_addr = tilemap_base_addr + cur_row * 32; // TODO: need to account for scroll

    std::printf("%06x ", tilemap_offs_addr);

    for (dual* ptr = &vram[tilemap_offs_addr]; ptr != &vram[tilemap_offs_addr + 32]; ++ptr) {
      SPPU::bg_map_tile_t* t = (SPPU::bg_map_tile_t*) ptr;
      if (t->char_no == 0xfc) {
        std::printf("     ");
      } else {
        std::printf("%-3x%c ", t->char_no, t->flip_x ? '*' : ' ');
      }
    }

    std::printf("\n");

    std::printf("%06x ", tilemap_offs_addr);
    for (dual* ptr = &vram[tilemap_offs_addr]; ptr != &vram[tilemap_offs_addr + 32]; ++ptr) {
      SPPU::bg_map_tile_t* t = (SPPU::bg_map_tile_t*) ptr;
      std::printf("%-4x ", t->reg);
    }

    std::printf("\n");
  }
}


void SPPU::render_row() {
  // get bg mode
  byte mode = bgmode.mode;
  byte bg = 2;

  byte bpp = bpps[bg]; // 2bpp means one pixel is encoded in one word
  bpp /= 2;

  // assume we're looking at 4bpp layer 0
  // get base address for this layer
  dword tilemap_base_addr = bg_base_size[bg].base_addr * 0x400;

  // 1024 words after this addr correspond to the current tilemap
  byte cur_row = line / 8;
  byte tile_row = line % 8;

  dword chr_base_addr = bg_chr_base_addr_for_bg(bg);

  dword tilemap_offs_addr = tilemap_base_addr + cur_row * 32; // TODO: need to account for scroll
//
//
////  std::printf("%06x\n", tilemap_offs_addr);
////  std::cout << line << ": ";
////  for (auto tile_id : tile_ids) {
////    std::cout << tile_id << " ";
////  }
////  std::cout << "\n";
//bool ok=false;
//  std::printf("%06x %3d ", tilemap_offs_addr, line);
//  for (auto tile_id : tile_ids) {
//    std::printf("%-3x ", tile_id);
//    if (tile_id == 0x13a) ok =true;
//  }
//  std::printf("\n");
//  if (vram[0x13b1].w != 0 && ok) {
//    ;
//  }

  // coming into this, we get 32 tile ids. for each tile id, we want to decode 8 palette values:
  int col = 0;
  std::for_each(&vram[tilemap_offs_addr], &vram[tilemap_offs_addr + 32],
                [&](dual bg_tile_data) {
                  SPPU::bg_map_tile_t* t = (SPPU::bg_map_tile_t*) &bg_tile_data;
                  auto tile_id = t->char_no;
                  auto* fb_ptr = screen->fb[0].data() + line * 256 + col * 8;

                  auto row_to_access = tile_row;
                  if (t->flip_y)
                    row_to_access = 7 - row_to_access;

                  // get tile chr data
                  word tile_chr_base = chr_base_addr + (8 * bpp) * tile_id + row_to_access;
//                  std::printf("> %06x (base %06x tile_id %04x tile_row %02x)\n", tile_chr_base,
//                      chr_base_addr, tile_id, tile_row);

                  // read bpp bytes (bpp/2 words)
                  std::vector<word> data { vram[tile_chr_base].w, vram[tile_chr_base+1].w };

                  // decode planar data
                  // produce 8 byte values (palette indices)
                  for (int i = 0; i < 8; ++i) {
                    byte pal_byte = decode_planar(&vram[tile_chr_base], bpp, t->flip_x);
                    colour_t rgb = lookup(pal_byte);

                    fb_ptr->r = rgb.r;
                    fb_ptr->g = rgb.g;
                    fb_ptr->b = rgb.b;
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

colour_t SPPU::lookup(byte i) {
  word rgb = pal[2 * i] + (pal[2 * i + 1] << 8);
  return { .r = rgb & 0x1f, .g = (rgb >> 5) & 0x1f, .b = (rgb >> 10) & 0x1f };
}

void SPPU::tick(byte master_cycles) {
  if (vram[0x50b3].w == 0x3898) {
    std::cout << ">><<\n";
    ;
  }
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
