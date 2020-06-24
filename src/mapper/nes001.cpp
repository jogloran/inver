//
// Created by Daniel Tse on 28/5/20.
//

#include "nes001.hpp"
#include "bus.hpp"

Mapper::Mirroring MMC1::get_mirroring() {
  return mirroring;
}

void MMC1::map_ram(const std::vector<char>& backup, size_t len) {
  std::copy_n(backup.begin(), ram.size(), ram.begin());
}

byte* MMC1::get_ram() {
  return ram.data();
}

void
MMC1::map(const std::vector<char>& data, byte prg_banks, byte chr_banks, const NESHeader* header) {
  rom.reserve(0x4000 * prg_banks);
  if (chr_banks == 0) {
    chr.resize(0x2000 * 8);
  } else {
    chr.reserve(0x2000 * chr_banks);
  }

  auto cur = flash((byte*) data.data(), 0x4000 * prg_banks, rom);
  flash(cur, 0x2000 * chr_banks, chr);
}

byte MMC1::read(word addr) {
  if (addr >= 0x6000 && addr <= 0x7fff) {
    // RAM (optional)
    return ram[addr - 0x6000];
  } else if (addr <= 0xbfff) { // 0x8000-0xbfff
    // ROM is mapping bfa7 -> 1bfa7
    if (prg_mode == PRGMode::Switch32KBAt0x8000) {
      return bank(prg_rom_bank & ~1)[addr - 0x8000];
    } else if (prg_mode == PRGMode::FirstBankAt0x8000) {
      return bank(0)[addr - 0x8000];
    } else {
      return bank(prg_rom_bank)[addr - 0x8000];
    }
  } else if (addr <= 0xffff) { // 0xc000-0xffff
    if (prg_mode == PRGMode::Switch32KBAt0x8000) {
      return bank(prg_rom_bank & ~1)[addr - 0x8000];
    } else if (prg_mode == PRGMode::FinalBankAt0xC000) {
      return bank(-1)[addr - 0xc000];
    } else {
      return bank(prg_rom_bank)[addr - 0xc000];
    }
  }

  return 0;
}

void MMC1::write(word addr, byte value) {
  if (addr >= 0x6000 && addr <= 0x7fff) {
    ram[addr - 0x6000] = value;
  } else if (addr >= 0x8000) {
    if (value & 0x80) {
      shift = 0x10;
    } else {
      bool register_full = shift & 1;

      shift = ((value & 1) << 4) | (shift >> 1);
      if (register_full) {
        mmc1_command(addr & 0x6000, shift);
        shift = 0x10;
      }
    }
  }
}

byte MMC1::chr_read(word addr) {
  if (addr <= 0x0fff || chr_mode == CHRMode::Switch8KBBank) {
    return chr_bank(ppu_0000_bank)[addr];
  } else {
    return chr_bank(ppu_1000_bank)[addr - 0x1000];
  }
}

void MMC1::chr_write(word addr, byte value) {
  if (addr <= 0x0fff || chr_mode == CHRMode::Switch8KBBank) {
    chr_bank(ppu_0000_bank)[addr] = value;
  } else {
    chr_bank(ppu_1000_bank)[addr - 0x1000] = value;
  }
}

void MMC1::reset() {
  shift = 0x10;
  mirroring = Mirroring::AAAA;
  prg_mode = PRGMode::FinalBankAt0xC000;
  chr_mode = CHRMode::Switch8KBBank;
  ppu_0000_bank = 0;
  ppu_1000_bank = 0;
  prg_rom_bank = 0;
  prg_ram_disabled = false;
}

void MMC1::connect(Bus* b) {
  Mapper::connect(b);
}

void MMC1::mmc1_command(word mmc1_addr, byte value) {
  switch (mmc1_addr) {
    case 0x0: // CHR/PRG-ROM bank mode, mirroring (0x8000-9fff)
    {
      switch (value & 3) {
        case 0:
          mirroring = Mirroring::AAAA;
          break;
        case 1:
          mirroring = Mirroring::BBBB;
          break;
        case 2:
          mirroring = Mirroring::V;
          break;
        case 3:
          mirroring = Mirroring::H;
          break;
      }
      chr_mode = value & (1 << 4) ? CHRMode::SwitchTwo4KBBanks : CHRMode::Switch8KBBank;
      switch ((value & 0b1100) >> 2) {
        case 0:
        case 1:
          prg_mode = PRGMode::Switch32KBAt0x8000;
          break;
        case 2:
          prg_mode = PRGMode::FirstBankAt0x8000;
          break;
        case 3:
          prg_mode = PRGMode::FinalBankAt0xC000;
          break;
      }
      break;
    }
    case 0x2000: // select CHR bank for PPU 0x0000 (0xa000-bfff)
    {
      ppu_0000_bank = value;
      if (chr_mode == CHRMode::Switch8KBBank) {
        ppu_0000_bank &= ~1;
      }
      break;
    }
    case 0x4000: // select CHR bank for PPU 0x1000 (0xc000-dfff)
    {
      if (chr_mode == CHRMode::SwitchTwo4KBBanks) {
        ppu_1000_bank = value;
      }
      break;
    }
    case 0x6000: // select 16KB PRG-ROM bank, PRG-RAM enable (0xe000-ffff)
    {
      prg_rom_bank = value & 0b1111;
      if (prg_mode == PRGMode::Switch32KBAt0x8000) {
        prg_rom_bank &= ~1;
      }
      prg_ram_disabled = value >> 4;
      break;
    }
  }
}

byte* MMC1::chr_bank(byte bank) {
  return chr.data() + bank * 0x1000;
}
