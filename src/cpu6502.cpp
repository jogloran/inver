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
