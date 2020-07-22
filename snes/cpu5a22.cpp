//
// Created by Daniel Tse on 9/7/20.
//
#include <iomanip>

#include "cpu5a22.hpp"
#include "ops_5a22.hpp"
#include "bus_snes.hpp"
#include "op_names_5a22.hpp"
#include "rang.hpp"

#include <gflags/gflags.h>

DECLARE_bool(dis);
DECLARE_bool(xx);
DECLARE_bool(dump_stack);

std::ostream& hex_byte(std::ostream& out) {
  return out << std::hex << std::setw(2) << std::setfill('0') << std::right;
}

std::ostream& hex_word(std::ostream& out) {
  return out << std::hex << std::setw(4) << std::setfill('0') << std::right;
}

std::ostream& hex_addr(std::ostream& out) {
  return out << std::hex << std::setw(6) << std::setfill('0') << std::right;
}

const char* to_6502_flag_string(byte f) {
  static char buf[9] = "________";
  static const char* flags = "NVmxDIZC";
  for (int i = 0; i < 8; ++i) {
    buf[7 - i] = (f & 1) ? flags[7 - i] : '_';
    f >>= 1;
  }
  return buf;
}

void CPU5A22::dump_pc() {
  std::cout << hex_addr << int(pc.addr) << ": "
            << hex_byte << int(bus->read(pc.addr)) << ' '
            << std::setw(24) << std::left << std::setfill(' ')
            << instruction_at_pc(*this);
}

std::ostream& CPU5A22::dump_stack(std::ostream& out) {
  out << "〖 ";
  for (word ptr = 0x1ff; ptr > sp; --ptr) {
    out << hex_byte << static_cast<int>(bus->read(ptr)) << ' ';
  }
  return out << "】";
}

byte last_mode = 0xff;

void CPU5A22::dump() {
  using std::hex;
  using std::dec;
  using std::setw;
  using std::setfill;
  using std::left;

  std::cout << " sp: " << hex_word << static_cast<int>(sp) << (!native() ? "•" : " ");
  std::cout << to_6502_flag_string(p.reg) << " ("
            << hex_byte << int(p.reg) << ')';
  std::cout << " a: ";
  if (p.m) {
    std::cout << rang::style::dim << rang::fg::gray << hex_byte << static_cast<int>(a >> 8)
              << rang::style::reset << rang::fg::reset
              << hex_byte << static_cast<int>(a & 0xff);
  } else {
    std::cout << hex_word << static_cast<int>(a);
  }
  if (p.x) {
    std::cout << " x: " << rang::style::dim << rang::fg::gray << hex_byte
              << static_cast<int>(x >> 8) << rang::style::reset << rang::fg::reset << hex_byte
              << static_cast<int>(x & 0xff)
              << " y: " << rang::style::dim << rang::fg::gray << hex_byte
              << static_cast<int>(y >> 8) << rang::style::reset << rang::fg::reset << hex_byte
              << static_cast<int>(y & 0xff);
  } else {
    std::cout << " x: " << hex_word << static_cast<int>(x)
              << " y: " << hex_word << static_cast<int>(y);
  }
//  std::cout << " APU: " << hex_byte << static_cast<int>(read(0x2140))
//            << static_cast<int>(read(0x2141))
//            << static_cast<int>(read(0x2142))
//            << static_cast<int>(read(0x2143));
  std::cout << " 0000:" << hex_byte << static_cast<int>(read(0x0000))
            << hex_byte << static_cast<int>(read(0x0001))
            << hex_byte << static_cast<int>(read(0x0002))
            << hex_byte << static_cast<int>(read(0x0003))
            << hex_byte << static_cast<int>(read(0x0004));
  std::cout << " mode:" << hex_byte << static_cast<int>(read(0x7e0100));
//  std::cout << " 1df5:" << hex_byte << static_cast<int>(read(0x1df5));
  std::cout << " nmi:" << hex_byte << static_cast<int>(bus->nmi.reg);
  std::cout << " cyc: " << std::dec << ncycles;

  if (FLAGS_dump_stack) {
    std::cout << " stk: ";
    dump_stack(std::cout);
  }

  std::cout << std::endl;
}

void CPU5A22::tick() {
  if (cycles_left == 0) {
    byte opcode = bus->read(pc.addr);

    if (FLAGS_dis) dump_pc();
    ++pc.addr;
    if (FLAGS_dis) dump();

    auto mode = bus->read(0x100);
//    std::printf("mode: %02x %02x\n", mode, last_mode);
    if (mode != last_mode) {
      last_mode = mode;
      std::printf("mode: %02x\n", last_mode);
    }

    auto op = ops_65c816[opcode];
    cycle_count_t extra_cycles = op.f(*this);
    cycles_left = op.cyc + extra_cycles;
  }

  --cycles_left;
  ++ncycles;
}

void CPU5A22::reset() {
  p.reg = 0b00110000;
  sp.w = 0x1ff;
}

void CPU5A22::connect(BusSNES* b) {
  bus = b;
}

void CPU5A22::write(dword address, byte value) {
  bus->write(address, value);
}

byte CPU5A22::read(dword address) {
  return bus->read(address);
}

byte CPU5A22::pop() {
  auto result = bus->ram[(sp & 0xff00) + (((sp & 0xff) + 1) & 0xff)];
  ++sp;
  return result;
}

word CPU5A22::pop_word() {
  auto result = (bus->ram[(sp & 0xff00) + (((sp & 0xff) + 1) & 0xff)] |
                 (bus->ram[(sp & 0xff00) + (((sp & 0xff) + 2) & 0xff)] << 8));
  ++sp;
  ++sp;
  return result;
}

void CPU5A22::push(byte data) {
  bus->ram[sp] = data;
  --sp;
}

void CPU5A22::push_word(word address) {
  bus->ram[(sp & 0xff00) + ((sp - 1) & 0xff)] = address & 0xff;
  bus->ram[(sp & 0xff00) + (sp & 0xff)] = address >> 8;
  --sp;
  --sp;
}

void CPU5A22::pop_flags() {
  if (native()) {
    p.reg = pop();
    if (p.x) {
      x.h = 0;
      y.h = 0;
    }
  } else {
    p.reg = (pop() & 0b11001111) | (p.reg & 0b00110000);
  }
}
