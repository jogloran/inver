
#include "mmu.h"
#include "cpu.h"

byte& MMU::vram(word loc, bool use_alt_bank) {
  if (!use_alt_bank) {
    return mem[loc];
  } else {
    return vram_bank_mem[loc - 0x8000];
  }
}

byte& MMU::operator[](word loc) {
  byte* result = mbc->get(loc);
  if (result != nullptr) {
    return *result;
  }
  
  result = apu.get(loc);
  if (result != nullptr) {
    return *result;
  }
  
  // TODO: ff03 should have the lower byte of timer div
  switch (loc) {
    case 0xff04: { // timer DIV
      return timer.div();
    }
    case 0xff05: { // timer counter
      return timer.counter;
    }
    case 0xff06: {
      return timer.modulo;
    }
    case 0xff07: {
      return timer.tac();
    }
    case 0xff00: { // joypad
      handle_joypad();
      mem[loc] = joypad;
      return joypad;
    }
    case 0xff69: {
      return bgp[bgp_index];
    }
    case 0xff6b: {
      return obp[obp_index];
    }
  }
  
  if (loc <= 0x00ff) {
    if (rom_mapped)
      return rom[loc];
    else
      return cart[loc];
  } else if (loc >= 0x8000 && loc <= 0x9fff) {
    if (vram_bank == 0) {
      return mem[loc];
    } else {
      return vram_bank_mem[loc - 0x8000];
    }
  } else if (loc >= 0xc000 && loc <= 0xcfff) {
    return cgb_ram[loc - 0xc000];
  } else if (loc >= 0xd000 && loc <= 0xdfff) {
    return cgb_ram[cgb_ram_bank * 0x1000 + (loc - 0xd000)];
  } else if (loc >= 0xe000 && loc <= 0xfdff) {
    return mem[loc - 0x2000];
  } else {
    return mem[loc];
  }
}

void MMU::set(word loc, byte value) {
  if (mbc->set(loc, value)) {
    return;
  }
  if (apu.set(loc, value)) {
    return;
  }
  
  if (loc >= 0x8000 && loc <= 0x9fff) {
    if (vram_bank == 0) {
      mem[loc] = value;
    } else {
      vram_bank_mem[loc - 0x8000] = value;
    }
    return;
  } else if (loc >= 0xc000 && loc <= 0xcfff) {
    cgb_ram[loc - 0xc000] = value;
    return;
  } else if (loc >= 0xd000 && loc <= 0xdfff) {
    cgb_ram[cgb_ram_bank * 0x1000 + (loc - 0xd000)] = value;
    return;
  }
  
  mem[loc] = value;
  
  switch (loc) {
      // serial write
    case 0xff01: {
      last = value;
      break;
    }
      // serial read
    case 0xff02: {
      //    std::cout << (char)last;
    }
      
    case 0xff04: { // timer DIV
      timer.reset_div();
      break;
    }
    case 0xff05: { // timer counter
      timer.counter = value;
      break;
    }
    case 0xff06: {
      timer.modulo = value;
      break;
    }
    case 0xff07: {
      timer.set_tac(value);
      break;
    }
    
    case 0xff4d: {
      if (value == 1) {
        ppu.cpu.speed_switch_prepare();
      }
      break;
    }
    
    case 0xff4f: { // vram bank switch
      vram_bank = value & 0x1;
      break;
    }
    
    case 0xff51:
    case 0xff52:
    case 0xff53:
    case 0xff54:
    case 0xff55: {
      std::cout << "hdma " << hex<<loc << "=" << int(value) << std::endl;
      break;
    }
    
    case 0xff68: {
      bgp_index = value & 0b00011111;
      bgp_auto_increment_on_write = value & (1 << 7);
      
      break;
    }
    case 0xff69: {
      // word values are stored little-endian (as usual)
      bgp[bgp_index] = value;
//      std::cout << "writing " << hex<<setfill('0')<<setw(2)<< int(value) << " <- " << int(bgp_index) << std::endl;
      if (bgp_auto_increment_on_write) bgp_index = (bgp_index + 1) % 64;
      
      break;
    }
    case 0xff6a: {
      obp_index = value & 0b00011111;
      obp_auto_increment_on_write = value & (1 << 7);
      break;
    }
    case 0xff6b: {
      obp[obp_index] = value;
      if (obp_auto_increment_on_write) obp_index = (obp_index + 1) % 64;
      break;
    }
    
    case 0xff70: {
//      std::cout << "ram bank switch " << hex<<loc << "=" << int(value) << std::endl;
      if (value == 0x0) {
        cgb_ram_bank = 1;
      } else if (value <= 0x7) {
        cgb_ram_bank = value;
      }
      break;
    }
      
    case 0xff40: { // LCD stat
      ppu.stat(value);
      break;
    }
      
    case 0xff00: { // joypad
      mem[loc] = value | 0xf;
      break;
    }
      
    case 0xff46: { // DMA
      word src = value << 8;
      //    for (word addr = 0xfe00; addr < 0xfea0; ++addr) {
      //      (*this)[addr] = (*this)[src++];
      //    }
      // This isn't valid if we are reading from "special"
      // memory locations (i.e. memory mapped I/O registers etc)
      std::copy_n(&((*this)[src]), 0xa0, &mem[0xfe00]);
      break;
    }
      
    case 0xff50: { // unmap rom
      rom_mapped = false;
      break;
    }
  }
}

void
MMU::handle_joypad() {
  input.poll();
  
  byte key_input = 0xf;
  
  byte value = mem[0xff00] | 0xf;
  if ((value & 0x20) == 0) {
    if ((input.state & Buttons::Start) == Buttons::Start) {
      key_input ^= 0x8;
    }
    if ((input.state & Buttons::Select) == Buttons::Select) {
      key_input ^= 0x4;
    }
    if ((input.state & Buttons::B) == Buttons::B) {
      key_input ^= 0x2;
    }
    if ((input.state & Buttons::A) == Buttons::A) {
      key_input ^= 0x1;
    }
  } else if ((value & 0x10) == 0) {
    if ((input.state & Buttons::D) == Buttons::D) {
      key_input ^= 0x8;
    }
    if ((input.state & Buttons::U) == Buttons::U) {
      key_input ^= 0x4;
    }
    if ((input.state & Buttons::L) == Buttons::L) {
      key_input ^= 0x2;
    }
    if ((input.state & Buttons::R) == Buttons::R) {
      key_input ^= 0x1;
    }
  }
  
  joypad = (0xf0 & value) | key_input;
}

byte
MMU::effective_rom_bank_for_loc(word loc) {
  if (loc <= 0x3fff) {
    return 0;
  } else {
    return mbc->bank_no();
  }
}
