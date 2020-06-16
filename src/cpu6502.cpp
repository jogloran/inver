#include <iostream>
#include <iomanip>
#include <gflags/gflags.h>

#include "cpu6502.hpp"

#include "ops.hpp"
#include "op_names.hpp"
#include "utils.hpp"

DECLARE_bool(dis);
DECLARE_bool(dump_stack);

constexpr byte INITIAL_STATUS_REG = 0x24;

[[maybe_unused]] void CPU6502::set_pc(word address) {
  pc = address;
}

void CPU6502::reset() {
  a = x = y = ncycles = cycles_left = 0;
  sp = 0xfd;
  pc = bus->read_vector<Bus::Interrupt::RST>();
  std::cerr << "setting pc to " << std::hex << pc << std::endl;
  p.reg = INITIAL_STATUS_REG;
  crossed_page = false;
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
  auto result = (bus->read(SP_BASE + ((sp + 1) & 0xff)) |
                 (bus->read(SP_BASE + ((sp + 2) & 0xff)) << 8));
  sp += 2;
  return result;
}

// Popping with an empty stack (sp = 0x1ff) must set sp = 0x00 and read from 0x100 instead
byte CPU6502::pop() {
  auto result = bus->read(SP_BASE + ((sp + 1) & 0xff));
  ++sp;
  return result;
}

bool CPU6502::irq() {
  if (!p.I) {
    push_word(pc);
    push(p.reg);
    p.B = 0;
    p.U = 1;
    p.I = 1;
    pc = bus->read_vector<Bus::Interrupt::IRQ>();

    cycles_left = 8;
    return true;
  }
  return false;
}

// The copy of P pushed by BRK should always contain __11 ____ (the unused bits set to 1)
// and with the I flag disabled _after_ pushing
void CPU6502::brk() {
  push_word(pc + 1);
  push((p.reg & ~0b00110000) | 0b00110000);
  p.I = 1;
  pc = bus->read_vector<Bus::Interrupt::IRQ>();
}

void
CPU6502::nmi() {
  push_word(pc);
  push(p.reg);
  p.B = 0;
  p.U = 1;
  p.I = 1;
  word handler = bus->read_vector<Bus::Interrupt::NMI>();;
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
  std::cout << " a: " << hex_byte << static_cast<int>(a)
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

std::ostream& CPU6502::dump_stack(std::ostream& out) {
  out << "[ ";
  for (word ptr = 0x1ff; ptr > SP_BASE + sp; --ptr) {
    out << hex_byte << static_cast<int>(bus->read(ptr)) << ' ';
  }
  return out << "]";
}
