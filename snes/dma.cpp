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
  dword dst = 0x2100 | B_addr;

  if (!in_transfer) goto proc;

  // switch according to dma_params.tx_type, per byte to transfer:
  //   read byte from das/hdma_ptr
  //   increment das/hdma_ptr
  //   write to B_addr + 0,1,2,3
  log("HDMA %-6s 0x%-6x <- indirect? %d line count: 0x%02x\n",
      dma_modes[dma_params.tx_type],
      dst, indirect, hdma_line_counter);
  switch (dma_params.tx_type) {
    case 0:
      hdma_write_unit(dst);
      break;
    case 1:
      hdma_write_unit(dst);
      hdma_write_unit(dst + 1);
      break;
    case 2:
    case 6:
      hdma_write_unit(dst);
      hdma_write_unit(dst);
      break;
    case 3:
    case 7:
      hdma_write_unit(dst);
      hdma_write_unit(dst);
      hdma_write_unit(dst + 1);
      hdma_write_unit(dst + 1);
      break;
    case 4:
      hdma_write_unit(dst);
      hdma_write_unit(dst + 1);
      hdma_write_unit(dst + 2);
      hdma_write_unit(dst + 3);
      break;
    case 5:
      hdma_write_unit(dst);
      hdma_write_unit(dst + 1);
      hdma_write_unit(dst);
      hdma_write_unit(dst + 1);
      break;
  }

  // At this stage, hdma_line_counter is already populated with a value:
  // v = 0x81-0xff:
  //   transfer one unit per scanline, v & 0x7f times
  // v = 0x01-0x80:
  //   transfer one unit, then pause for v - 1 scanlines (set hdma_wake_counter = v - 1)
  // v = 0x00:
  //   terminate this channel for this frame

  // decrement hdma_line_counter
proc:
  --hdma_line_counter;
  log("decremented line: %02x\n", hdma_line_counter);

  in_transfer = hdma_line_counter & 0x80;

  if ((hdma_line_counter & 0x7f) == 0) {
    dword hdma_addr = a1.addr; // The value here is constant, unlike in DMA where it's a counter
    byte bank = hdma_addr >> 16;
//    hdma_ptr = hdma_addr; // truncate the bank number. hdma_ptr is the one that changes

    // load in the table data into the registers
    byte instr = bus->read(hdma_ptr++ | (bank << 16));
    log("read from 0x%06x -> %02x\n", ((hdma_ptr - 1) | (bank << 16)), instr);
    if (instr != 0) {
      hdma_line_counter = instr;
      in_transfer = true;

    } else {
      hdma_enabled = false;
    }

    // if indirect, then treat the two bytes after the number of lines
    // as a pointer to the data units to transfer
    if (indirect) {
      das.lo = bus->read(hdma_ptr++ | (bank << 16));
      das.md = bus->read(hdma_ptr++ | (bank << 16));

      log("indirect reading from %06x\n", das.addr);
    }

  }
}

void DMA::hdma_init(bool indirect) {
  if (!hdma_enabled) return;

  dword hdma_addr = a1.addr; // The value here is constant, unlike in DMA where it's a counter

  byte bank = hdma_addr >> 16;
  hdma_ptr = hdma_addr; // truncate the bank number. hdma_ptr is the one that changes

  // load in the table data into the registers
  byte instr = bus->read(hdma_ptr++ | (bank << 16));
  log("init: read from 0x%06x -> %02x\n", ((hdma_ptr - 1) | (bank << 16)), instr);
  if (instr != 0) {
    hdma_line_counter = instr;
    in_transfer = true;

  } else {
    hdma_enabled = false;
  }

  // if indirect, then treat the two bytes after the number of lines
  // as a pointer to the data units to transfer
  if (indirect) {
    das.lo = bus->read(hdma_ptr++ | (bank << 16));
    das.md = bus->read(hdma_ptr++ | (bank << 16));

    log("init: indirect reading from %06x\n", das.addr);
  }

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

byte DMA::hdma_fetch() {
  if (dma_params.hdma_indirect_table) {
    auto value = bus->read(das.addr++);
    return value;
  } else {
    dword hdma_addr = a1.addr;
    byte bank = hdma_addr >> 16;
    return bus->read(hdma_ptr++ | (bank << 16));
  }
}

void DMA::hdma_write_unit(dword dst) {
  byte datum = hdma_fetch();
  if (dma_params.hdma_indirect_table) {
    log("0x%-6x <- 0x%04x [%02x]\n", dst, das.addr - 1, datum);
  } else {
    dword hdma_addr = a1.addr;
    byte bank = hdma_addr >> 16;
    log("0x%-6x <- 0x%04x [%02x]\n", dst, (hdma_ptr - 1) | (bank << 16), datum);
  }
  bus->write(dst, datum);
}

void DMA::log_write_unit(dword src, dword dst, byte value) {
#ifndef NDEBUG
  auto* maybe_vram_address = update_vram_address(dst, bus->ppu.get());
  log("\tdst %06x <- %06x [%-6s -> %02x] (0x%x bytes left)\n", dst, src, maybe_vram_address,
      value, das.addr & 0xffff);
#endif
}
