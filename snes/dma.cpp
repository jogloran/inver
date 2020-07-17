#include "dma.hpp"
#include "bus_snes.hpp"

cycle_count_t DMA::run() {
  // while length counter > 0
  // transfer one unit from source to destination
  // decrement length counter (43x5,6)
  cycle_count_t cycles {0};
  if (enabled) {
    dword& src = a1.addr;
    dword dst = 0x2100 | B_addr;
    if (dma_params.b_to_a) std::swap(src, dst);

    auto incr = A_step[dma_params.dma_A_step];
    log("DMA transfer mode %d\n", dma_params.tx_type);
    while ((das.addr & 0xffff) > 0) { // TODO: what if addr < 4 and we transfer 4 bytes?
      log("DMA dst %06x <- %06x [%02x] (0x%x bytes left)\n", dst, src, bus->read(src), das.addr & 0xffff);
      switch (dma_params.tx_type) {
        case 0:
          bus->write(dst, bus->read(src)); --das.addr;
          break;
        case 1:
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst + 1, bus->read(src)); --das.addr;
          break;
        case 2:
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst, bus->read(src)); --das.addr;
          break;
        case 3:
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst + 1, bus->read(src)); --das.addr;
          bus->write(dst + 1, bus->read(src)); --das.addr;
          break;
        case 4:
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst + 1, bus->read(src)); --das.addr;
          bus->write(dst + 2, bus->read(src)); --das.addr;
          bus->write(dst + 3, bus->read(src)); --das.addr;
          break;
        case 5:
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst + 1, bus->read(src)); --das.addr;
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst + 1, bus->read(src)); --das.addr;
          break;
        case 6:
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst, bus->read(src)); --das.addr;
          break;
        case 7:
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst, bus->read(src)); --das.addr;
          bus->write(dst + 1, bus->read(src)); --das.addr;
          bus->write(dst + 1, bus->read(src)); --das.addr;
          break;
      }
      a1.addr += incr;
    }

    enabled = false;
  }

  return cycles;
}
