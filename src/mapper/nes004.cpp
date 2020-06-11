#include "nes004.hpp"
#include "bus.hpp"
#include "cpu6502.hpp"

Mapper::Mirroring MMC3::get_mirroring() {
  return mirroring_mode;
}

void MMC3::signal_scanline() {
  if (irq_counter == 0) {
    irq_counter = irq_period;
  } else --irq_counter;

  // this happens upon the transition from irq_counter == 1 to 0
  if (irq_enabled && irq_counter == 0) {
    irq_requested_ = true;
  }
}

bool MMC3::irq_requested() {
  return irq_requested_;
}

void MMC3::irq_handled() {}

void MMC3::irq_enable(bool enable) {
  if (!enable) {
    irq_requested_ = false;
  }

  irq_enabled = enable;
}

void MMC3::write(word addr, byte value) {
  if (addr >= 0x6000 && addr <= 0x7fff) {
    ram[addr - 0x6000] = value;
  } else if (addr >= 0x8000 && addr <= 0x9fff) {
    if (addr & 1) { // bank select
      if (target_bank <= 1) value &= ~1;
      if (target_bank >= 6) value &= 0x3f;
//      if (bank_for_target[target_bank] != value && target_bank <= 5) {
//        log("R%d -> %02x\n", target_bank, value);
//      }
      bank_for_target[target_bank] = value;
    } else { // target select
      target_bank = value & 0b111;
      rom_8000_fixed = value & (1 << 6);
      chr_a12_inversion = value & (1 << 7);
    }
  } else if (addr <= 0xbfff) {
    if (addr & 1) { // PRG-RAM protect

    } else { // mirroring
      mirroring_mode = value & 1 ? Mirroring::H : Mirroring::V;
    }
  } else if (addr <= 0xdfff) {
    if (addr & 1) { // IRQ reload
      // Writing here will reset the IRQ counter and push the value from the latch
      // upon the next scanline (or cycle 260 of this scanline??)
      irq_counter = 0; // trigger a reload of irq_counter next scanline
    } else { // IRQ latch
      // Value written here will be loaded into the IRQ counter either when:
      // - the counter reaches 0
      // - IRQ reload occurs
      irq_period = value;
    }
  } else {
    irq_enable(addr & 1);
  }
}

void MMC3::map_ram(const std::vector<char>& backup, size_t len) {
  std::copy_n(backup.begin(), ram.size(), ram.begin());
}

byte* MMC3::get_ram() {
  return ram.data();
}
