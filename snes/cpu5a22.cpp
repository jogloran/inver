//
// Created by Daniel Tse on 9/7/20.
//
#include <iomanip>

#include "bus_snes.hpp"
#include "cpu5a22.hpp"
#include "debug.hpp"
#include "op_names_5a22.hpp"
#include "ops_5a22.hpp"
#include "rang.hpp"

#include <gflags/gflags.h>
#include <map>

DECLARE_bool(dis);
DECLARE_bool(dis_dump_m7);
DECLARE_bool(dis_raw);
DECLARE_bool(xx);
DECLARE_bool(dump_stack);

extern std::map<dword, PCWatchSpec> dis_pcs;
extern std::set<dword> ignored_pcs;
extern std::vector<ChangeWatchSpec> change_watches;

std::ostream& hex_byte(std::ostream& out) {
  return out << std::hex << std::setw(2) << std::setfill('0') << std::right;
}

std::ostream& hex_word(std::ostream& out) {
  return out << std::hex << std::setw(4) << std::setfill('0') << std::right;
}

std::ostream& hex_addr(std::ostream& out) {
  return out << std::hex << std::setw(6) << std::setfill('0') << std::right;
}

template <typename T>
std::ostream& hex_word_alternating(std::ostream& out, bool dim, T w) {
  //  if (!dim) {
  //    out << rang::style::reset << rang::fg::reset;
  //  } else {
  //    out << rang::style::dim << rang::fg::gray;
  //  }
  return out << hex_word << w;
}

template <typename T, typename... Args>
std::ostream& hex_word_alternating(std::ostream& out, bool dim, T w, Args... args) {
  hex_word_alternating(out, dim, w);
  hex_word_alternating(out, !dim, args...);
  //  out << rang::style::reset << rang::fg::reset;
  return out;
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

void CPU5A22::dump_m7(bool flush) {
  std::cout << "abcd ";
  hex_word_alternating(std::cout, false,
                       bus->ppu->m7.a(),
                       bus->ppu->m7.b(),
                       bus->ppu->m7.c(),
                       bus->ppu->m7.d());
  std::cout << " hvxy ";
  hex_word_alternating(std::cout, false,
                       bus->ppu->m7.h.w,
                       bus->ppu->m7.v.w,
                       bus->ppu->m7.x0(),
                       bus->ppu->m7.y0());
  if (flush) {
    std::cout << std::endl;
  }
}

std::ostream& CPU5A22::dump_stack(std::ostream& out) {
  out << "〖 ";
  for (word ptr = 0x1fff; ptr > sp; --ptr) {
    out << hex_byte << static_cast<int>(bus->read(ptr)) << ' ';
  }
  return out << "】";
}

void CPU5A22::dump() {
  using std::dec;
  using std::hex;
  using std::left;
  using std::setfill;
  using std::setw;

  std::cout << " sp: " << hex_word << static_cast<int>(sp) << (!native() ? "•" : " ");
  std::cout << to_6502_flag_string(p.reg) << " ("
            << hex_byte << int(p.reg) << ')';
  std::cout << " a: ";
  if (p.m) {
    if (!FLAGS_dis_raw) {
      std::cout << rang::style::dim << rang::fg::gray << hex_byte << static_cast<int>(a >> 8)
                << rang::style::reset << rang::fg::reset;
    }
    std::cout << hex_byte << static_cast<int>(a & 0xff);
  } else {
    std::cout << hex_word << static_cast<int>(a);
  }
  if (p.x) {
    std::cout << " x: ";
    if (!FLAGS_dis_raw) {
      std::cout << rang::style::dim << rang::fg::gray;
    }
    std::cout << hex_byte
              << static_cast<int>(x >> 8);
    if (!FLAGS_dis_raw) {
      std::cout << rang::style::reset << rang::fg::reset;
    }
    std::cout << hex_byte
              << static_cast<int>(x & 0xff);
    std::cout << " y: ";
    if (!FLAGS_dis_raw) {
      std::cout << rang::style::dim << rang::fg::gray;
    }
    std::cout << hex_byte
              << static_cast<int>(y >> 8);
    if (!FLAGS_dis_raw) {
      std::cout << rang::style::reset << rang::fg::reset;
    }
    std::cout << hex_byte
              << static_cast<int>(y & 0xff);
  } else {
    std::cout << " x: " << hex_word << static_cast<int>(x)
              << " y: " << hex_word << static_cast<int>(y);
  }

  if (FLAGS_dis_dump_m7) {
    std::cout << ' ';
    dump_m7();
  }

  std::cout << " nmi:" << hex_byte << static_cast<int>(bus->nmi.reg);
  std::cout << " cyc: " << std::dec << ncycles;
  std::cout << " y: " << bus->ppu->line;

  if (FLAGS_dump_stack) {
    std::cout << " stk: ";
    dump_stack(std::cout);
  }

  std::cout << std::endl;
}

void CPU5A22::tick() {
  if (cycles_left == 0) {
    byte opcode = bus->read(pc.addr);

    bool dis_here = false;
    bool ignored = false;
#ifndef NDEBUG
    auto dis_pc_cmd = dis_pcs.find(pc.addr);
    if (dis_pc_cmd != dis_pcs.end() && (dis_pc_cmd->second.action & PCWatchSpec::X)) {
      if (dis_pc_cmd->second.log_from_here)
        FLAGS_dis = true;
      dis_here = true;
    }

    ignored = ignored_pcs.find(pc.addr) != ignored_pcs.end();
#endif
    if ((FLAGS_dis || dis_here) && !ignored) dump_pc();

    ++pc.addr;

    if ((FLAGS_dis || dis_here) && !ignored) dump();
#ifndef NDEBUG
    for (ChangeWatchSpec& spec : change_watches) {
      spec(bus->read(spec.addr));
    }
#endif

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

byte CPU5A22::pop() {
  byte result;
  if (native()) {
    result = bus->ram[sp + 1];
  } else {
    result = bus->ram[(sp & 0xff00) + (((sp & 0xff) + 1) & 0xff)];
  }
  ++sp;
  return result;
}

word CPU5A22::pop_word() {
  word result;
  if (native()) {
    result = (bus->ram[sp + 1] | (bus->ram[sp + 2] << 8));
  } else {
    result = (bus->ram[(sp & 0xff00) + (((sp & 0xff) + 1) & 0xff)] |
              (bus->ram[(sp & 0xff00) + (((sp & 0xff) + 2) & 0xff)] << 8));
  }
  ++sp;
  ++sp;
  return result;
}

void CPU5A22::push(byte data) {
  bus->ram[sp] = data;
  --sp;
}

void CPU5A22::push_word(word address) {
  if (native()) {
    bus->ram[sp - 1] = address & 0xff;
    bus->ram[sp] = address >> 8;
  } else {
    bus->ram[(sp & 0xff00) + ((sp - 1) & 0xff)] = address & 0xff;
    bus->ram[(sp & 0xff00) + (sp & 0xff)] = address >> 8;
  }
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

int CPU5A22::bcd_add(int x, int addend) {
  // assume x, addend < 0x9999
  int out = 0;
  int carry = 0;
  int bits = 0;

  int out_digit = 0;
  while (x != 0 || addend != 0) {
    auto x_digit = x & 0xf;
    auto addend_digit = addend & 0xf;

    // assume x_digit, addend_digit < 0x9
    out_digit = x_digit + addend_digit + carry;
    carry = 0;
    if (out_digit > 0x9) {
      carry = 1;
      out_digit -= 10;
    }

    out |= (out_digit << bits);

    x >>= 4;
    addend >>= 4;
    bits += 4;
  }

  out |= (carry << bits);

  return out;
}
