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
    snprintf(maybe_vram_address, 8, "《%04x》", ppu->oamadd.addr);
  }
  if (dst == 0x2122) {
    snprintf(maybe_vram_address, 8, "{%04x}", ppu->cgram_addr);
  }

  return maybe_vram_address;
}

cycle_count_t DMA::hdma_init() {
  hdma_init(dma_params.hdma_indirect_table);
  return 0;
}

void DMA::hdma_tick() {
  // Called once per scanline at start of hblank
  if (!hdma_enabled) return;

  bool indirect = dma_params.hdma_indirect_table;

  // Transfer one "unit" of data according to dma_params.tx_type
  // from A -> B or B -> A according to dma_params.b_to_a
  // a1 <-> (0x2100 | B_addr)
  // if indirect,
  if (in_transfer) {
    dword dst = 0x2100 | B_addr;
    byte bank = a1.hi;

    // read byte at hdma_ptr as an instruction, then copy one "unit" of data (depending on the dma_params.tx_type)

    if ((hdma_line_counter & 0x80) == 0) {
      if ((hdma_line_counter & 0x7f) > 0) {
        log("here: %d\n", hdma_line_counter);
        --hdma_line_counter;
        return;
      }
    } else {
      --hdma_line_counter;
    }

    log("HDMA %-6s 0x%-6x reading from 0x%02x%04x line count: %d\n",
        dma_modes[dma_params.tx_type], dst, bank, hdma_ptr, hdma_line_counter);


    byte value;
    log("tx type: %d indirect: %d\n",dma_params.tx_type,indirect);
    switch (dma_params.tx_type) {
      case 0: {
        // assume A -> B to begin with
        if (indirect) {
          log("%04x <- %06x [%02x]\n", dst, das.addr, bus->read(das.addr));
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
        }
        bus->write(dst, value);
        break;
      }
      case 1: {
        if (indirect) {
          log("%04x <- %06x [%02x]\n", dst, das.addr, bus->read(das.addr));
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
        }
        bus->write(dst, value);
        if (indirect) {
          log("%04x <- %06x [%02x]\n", dst + 1, das.addr, bus->read(das.addr));
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
        }
        bus->write(dst + 1, value);
        break;
      }
      case 2: {
        if (indirect) {
          log("%04x <- %06x [%02x] (line = %d)\n", dst, das.addr, bus->read(das.addr), bus->ppu->line);
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
        }
        bus->write(dst, value);
        if (indirect) {
          log("%04x <- %06x [%02x]\n", dst, das.addr, bus->read(das.addr));
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
        }
        bus->write(dst, value);
        break;
      }
      case 3: {
        if (indirect) {
          log("%04x <- %06x [%02x] (line = %d)\n", dst, das.addr, bus->read(das.addr), bus->ppu->line);
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
          log("%04x <- [%02x]\n", dst, value);
        }
        bus->write(dst, value);
        if (indirect) {
          log("%04x <- %06x [%02x]\n", dst, das.addr, bus->read(das.addr));
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
          log("%04x <- [%02x]\n", dst, value);
        }
        bus->write(dst, value);
        if (indirect) {
          log("%04x <- %06x [%02x]\n", dst + 1, das.addr, bus->read(das.addr));
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
          log("%04x <- [%02x]\n", dst + 1, value);
        }
        bus->write(dst + 1, value);
        if (indirect) {
          log("%04x <- %06x [%02x]\n", dst + 1, das.addr, bus->read(das.addr));
          value = bus->read(das.addr++);
        } else {
          value = bus->read((bank << 16) | hdma_ptr++);
          log("%04x <- [%02x]\n", dst + 1, value);
        }
        bus->write(dst + 1, value);
        break;
      }
    }


    // in_transfer is true while we are going through the hdma table

    // this is true regardless of whether continue bit is set or not, when counter = 0
    if ((hdma_line_counter & 0x7f) == 0) {
      byte instr = bus->read(hdma_ptr++ | (bank << 16));
      if (instr != 0) {
        hdma_line_counter = instr;
      } else {
        hdma_enabled = false;
        in_transfer = false;
        return;
      }

      if (indirect) {
        das.lo = bus->read(hdma_ptr++ | (bank << 16));
        das.md = bus->read(hdma_ptr++ | (bank << 16));
      }

      in_transfer = true;
    }
  }
}

void DMA::hdma_init(bool indirect) {
  dword hdma_addr = a1.addr; // The value here is constant, unlike in DMA where it's a counter

  byte bank = hdma_addr >> 16;
  hdma_ptr = hdma_addr; // truncate the bank number. hdma_ptr is the one that changes

  // load in the table data into the registers
  byte instr = bus->read(hdma_ptr++ | (bank << 16));
  if (instr != 0) {
    hdma_line_counter = instr;
  } else {
    hdma_enabled = false;
  }

  if (indirect) {
    das.lo = bus->read(hdma_ptr++ | (bank << 16));
    das.md = bus->read(hdma_ptr++ | (bank << 16));

    log("indirect reading from %06x\n", das.addr);
  }

  in_transfer = true;
}



cycle_count_t DMA::run() {
  // while length counter > 0
  // transfer one unit from source to destination
  // decrement length counter (43x5,6)
  cycle_count_t cycles {0};
  if (dma_enabled) {
    dword& src = a1.addr;
    dword dst = 0x2100 | B_addr;
    if (dma_params.b_to_a) std::swap(src, dst);

    bool suppressed = false;
    dword last_src = 0;
    dword last_dst = 0;
    byte last_value = 0;
    dword skipped_since = 0;

    const char* maybe_vram_address = update_vram_address(dst, bus->ppu.get());

    auto incr = A_step[dma_params.dma_A_step];
    log("DMA %-6s 0x%-6x %-7s <- 0x%-6x (len 0x%-4x) pc: %06x incr: %d (%02x)\n",
        dma_modes[dma_params.tx_type],
        dst, maybe_vram_address, src, das.addr, bus->cpu->pc.addr,
        static_cast<int>(incr), dma_params.dma_A_step);

    if (das.addr == 0) das.addr = 0x10000;
    while ((das.addr & 0xffff) > 0) { // TODO: what if addr < 4 and we transfer 4 bytes?
      switch (dma_params.tx_type) {
        case 0:
          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;
          break;

        case 1:
          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst + 1, incr);
          if (das.addr == 0) goto out;
          break;

        case 2:
        case 6:
          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;
          break;

        case 3:
        case 7:
          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst + 1, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst + 1, incr);
          if (das.addr == 0) goto out;
          break;

        case 4:
          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst + 1, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst + 2, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst + 3, incr);
          if (das.addr == 0) goto out;
          break;

        case 5:
          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst + 1, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst, incr);
          if (das.addr == 0) goto out;

          write_unit(src, dst + 1, incr);
          if (das.addr == 0) goto out;
          break;
      }
    }
    out:;

    dma_enabled = false;
  }

  return cycles;
}

void DMA::write_unit(const dword& src, dword dst, sbyte incr) {
  byte value = bus->read(src);
  log_write_unit(src, dst, value);
  bus->write(dst, bus->read(src));
  a1.addr += incr;
  --das.addr;
}

void DMA::log_write_unit(dword src, dword dst, byte value) {
#ifndef NDEBUG
  auto* maybe_vram_address = update_vram_address(dst, bus->ppu.get());
  log("\tdst %06x <- %06x [%-6s -> %02x] (0x%x bytes left)\n", dst, src, maybe_vram_address,
      value, das.addr & 0xffff);
#endif
}
