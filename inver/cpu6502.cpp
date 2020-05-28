#include <iostream>
#include <iomanip>
#include <gflags/gflags.h>

#include "cpu6502.hpp"

#include "ops.hpp"
#include "op_names.hpp"

DECLARE_bool(dis);
DECLARE_bool(dump_stack);

constexpr byte INITIAL_STATUS_REG = 0x24;

[[maybe_unused]] void CPU6502::set_pc(word address) {
  pc = address;
}


void CPU6502::reset() {
  a = x = y = 0;
  sp = 0xfd;
  pc = bus->read(0xfffc) | (bus->read(0xfffd) << 8);
  std::cerr << "setting pc to " << std::hex << pc << std::endl;
  p.reg = INITIAL_STATUS_REG;
  ncycles = 0;
  cycles_left = 0;
  crossed_page = false;
}

const char *to_6502_flag_string(byte f) {
  static char buf[9] = "________";
  static const char *flags = "NVstDIZC";
  for (int i = 0; i < 8; ++i) {
    buf[7 - i] = (f & 1) ? flags[7 - i] : '_';
    f >>= 1;
  }
  return buf;
}

std::ostream& hex_byte(std::ostream& out) {
  return out << std::hex << std::setw(2) << std::setfill('0');
}

std::ostream& hex_word(std::ostream& out) {
  return out << std::hex << std::setw(4) << std::setfill('0');
}

std::ostream& CPU6502::dump_stack(std::ostream& out) {
  out << "[ ";
  for (word ptr = 0x1ff; ptr > SP_BASE + sp; --ptr) {
    out << hex_byte << static_cast<int>(bus->read(ptr)) << ' ';
  }
  return out << "]";
}

void CPU6502::tick() {
  if (cycles_left == 0) {
    byte opcode = bus->read(pc);

    if (FLAGS_dis && should_dump) dump_pc();
    ++pc;
    if (FLAGS_dis && should_dump) dump();

    auto op = ops[opcode];
    cycle_count_t extra_cycles = op.f(*this);
    cycles_left = op.cyc + extra_cycles;
  }

  --cycles_left;
  ++ncycles;
}

void CPU6502::push_word(word address) {
  bus->write(SP_BASE + sp - 1, address & 0xff);
  bus->write(SP_BASE + sp, address >> 8);
  sp -= 2;
}

void CPU6502::push(byte data) {
  bus->write(SP_BASE + sp, data);
  --sp;
}

void CPU6502::rts() {
  // Note the + 1 for the special behaviour of RTS
  pc = pop_word() + 1;
}

word CPU6502::pop_word() {
  auto result = (bus->read(SP_BASE + sp + 1) | (bus->read(SP_BASE + sp + 2) << 8));
  sp += 2;
  return result;
}

byte CPU6502::pop() {
  auto result = bus->read(SP_BASE + sp + 1);
  ++sp;
  return result;
}

word CPU6502::read_word() {
  word result = bus->read(pc++);
  return result | (bus->read(pc++) << 8);
}

cycle_count_t CPU6502::branch_with_offset() {
  sbyte offset = static_cast<sbyte>(bus->read(pc++));
  pc += offset;
  return ((pc - offset) & 0xff00) == (pc & 0xff00) ? 0 : 1;
}

bool CPU6502::irq() {
  if (!p.I) {
    push_word(pc);
    p.B = 0;
    p.U = 1;
    p.I = 1;
    push(p.reg);
    word handler = bus->read(0xfffe) | (bus->read(0xffff) << 8);
    pc = handler;

    cycles_left = 8;
    return true;
  }
  return false;
}

void
CPU6502::nmi() {
  push_word(pc);
  p.B = 0;
  p.U = 1;
  p.I = 1;
  push(p.reg);
  word handler = bus->read(0xfffa) | (bus->read(0xfffb) << 8);
  pc = handler;

  cycles_left = 8;
}

void CPU6502::dump() {
  using std::hex;
  using std::dec;
  using std::setw;
  using std::setfill;
  using std::left;

  std::cout << " sp: " << hex_byte << static_cast<int>(sp) << ' ';
  std::cout << to_6502_flag_string(p.reg) << " ("
            << hex_byte << int(p.reg) << ')';
  std::cout << std::right
            << " a: " << hex_byte << static_cast<int>(a)
            << " x: " << hex_byte << static_cast<int>(x)
            << " y: " << hex_byte << static_cast<int>(y);
  std::cout << " cyc: " << dec << setw(8) << setfill(' ') << (ncycles * 3) % 341
//            << " (0)+y: " << hex_word << (bus->read(0) | (bus->read(1) << 8)) + y << " = "
//            << hex_byte << static_cast<int>(bus->read((bus->read(0) | (bus->read(1) << 8)) + y)) << ' '
            << "(0),(1): " << hex_byte << static_cast<int>(bus->read(0x0))
            << hex_byte << static_cast<int>(bus->read(0x1)) << ' ';

  if (FLAGS_dump_stack) {
    std::cout << " stk: ";
    dump_stack(std::cout);
  }

  std::cout << std::endl;
}

void CPU6502::dump_pc() {
  std::cout << hex_word << pc << ": "
            << hex_byte << int(bus->read(pc)) << ' '
            << std::setw(24) << std::left << std::setfill(' ')
            << instruction_at_pc(*this);
}
