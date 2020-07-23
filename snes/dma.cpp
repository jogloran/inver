#include "dma.hpp"
#include "bus_snes.hpp"
#include "cpu5a22.hpp"

void DMA::log(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Logger<DMA>::log_with_tag(tag, msg, args);
  va_end(args);
}

constexpr const char* dma_modes[] = {
    "0", "0,1", "0,0", "0,0,1,1", "0,1,2,3", "0,1,0,1", "0,0", "0,1,0,1"
};

static const char* update_vram_address(dword dst, SPPU* ppu) {
  static char maybe_vram_address[8] = {};

  if (dst == 0x2118 || dst == 0x2119) {
    snprintf(maybe_vram_address, 8, "[%04x]", ppu->vram_addr.w);
  }
  if (dst == 0x2104) {
    snprintf(maybe_vram_address, 8, "<%04x>", ppu->oamadd.addr);
  }
  if (dst == 0x2122) {
    snprintf(maybe_vram_address, 8, "{%04x}", ppu->cgram_addr);
  }

  return maybe_vram_address;
}

cycle_count_t DMA::run() {
  // while length counter > 0
  // transfer one unit from source to destination
  // decrement length counter (43x5,6)
  cycle_count_t cycles {0};
  if (enabled) {
    dword& src = a1.addr;
    dword dst = 0x2100 | B_addr;
    if (dma_params.b_to_a) std::swap(src, dst);

    bool suppressed = false;
    dword last_src = 0;
    dword last_dst = 0;
    byte last_value = 0;
    dword skipped_since = 0;

    const char* maybe_vram_address = update_vram_address(dst, &bus->ppu);

    auto incr = A_step[dma_params.dma_A_step];
    log("DMA %-6s 0x%-6x %-7s <- 0x%-6x (len 0x%-4x) pc: %06x incr: %d (%02x)\n", dma_modes[dma_params.tx_type],
        dst, maybe_vram_address, src, das.addr, bus->cpu->pc.addr,
        static_cast<int>(incr), dma_params.dma_A_step);

    if (das.addr == 0) das.addr = 0x10000;
    while ((das.addr & 0xffff) > 0) { // TODO: what if addr < 4 and we transfer 4 bytes?
//      bool same =
//          dst == last_dst && value == last_value && (src == last_src || src == last_src + 1);
//      if (suppressed && same) { ;
//      } else {
//        log("\tdst %06x <- %06x [%02x] (0x%x bytes left)\n", dst, src, value, das.addr & 0xffff);
////        if (dst == 0x2118 || dst == 0x2119) {
////          log("vram address before: %04x\n", bus->ppu.vram_addr.w);
////        }
//        if (suppressed && !same) suppressed = false;
//        else if (!suppressed && same) {
//          log("\t...\n");
//          suppressed = true;
//          skipped_since = das.addr;
//        }
//      }

//      last_dst = dst;
//      last_src = src;
//      last_value = value;

      byte value;
      switch (dma_params.tx_type) {
        case 0:
          value = bus->read(src);
          maybe_vram_address = update_vram_address(dst, &bus->ppu);
          log("\tdst %06x <- %06x [%-6s -> %02x] (0x%x bytes left)\n", dst, src, maybe_vram_address, value, das.addr & 0xffff);
          bus->write(dst, bus->read(src));
          a1.addr += incr;
          --das.addr;
          if (das.addr == 0) goto out;
          break;
        case 1:
          value = bus->read(src);
          maybe_vram_address = update_vram_address(dst, &bus->ppu);
          log("\tdst %06x <- %06x [%-6s -> %02x] (0x%x bytes left)\n", dst, src, maybe_vram_address, value, das.addr & 0xffff);
          bus->write(dst, bus->read(src));
          a1.addr += incr;
          --das.addr;
          if (das.addr == 0) goto out;

          value = bus->read(src);
          maybe_vram_address = update_vram_address(dst, &bus->ppu);
          log("\tdst %06x <- %06x [%-6s -> %02x] (0x%x bytes left)\n", dst + 1, src, maybe_vram_address, value, das.addr & 0xffff);
          bus->write(dst + 1, bus->read(src));
          a1.addr += incr;
          --das.addr;
          if (das.addr == 0) goto out;
          break;
        case 2:
          // TODO: these need to be rewritten like case 1
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          break;
        case 3:
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 1, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 1, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          break;
        case 4:
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 1, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 2, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 3, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          break;
        case 5:
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 1, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 1, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          break;
        case 6:
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          break;
        case 7:
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 1, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          bus->write(dst + 1, bus->read(src));
          --das.addr;
          if (das.addr == 0) goto out;
          break;
      }
    }
    out:;

    enabled = false;
  }

  return cycles;
}
