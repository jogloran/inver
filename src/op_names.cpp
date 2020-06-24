#include "op_names.hpp"

std::array<Op, 256> op_meta = build_ops();

const char* addr_mode_templates[12] = {
    "(0x00%02x + X)", "0x00%02x", "$%02x", "0x%04x", "(0x00%02x) + Y", "0x00%02x + X", "0x%04x + Y",
    "0x%04x + X", "", "", "invalid", "+%2x"
};

const char* instrs[72] = {
    "ORA", "AND", "EOR", "ADC", "STA", "LDA", "CMP", "SBC",
    "ASL", "ROL", "LSR", "ROR", "STX", "LDX", "DEC", "INC",
    "invalid", "BIT", "JMP", "JMP (abs)", "STY", "LDY", "CPY", "CPX",

    // 16 0xX0 instructions
    "BRK", "BPL", "JSR", "BMI", "RTI", "BVC", "RTS", "BVS", "invalid", "BCC", nullptr, "BCS",
    nullptr, "BNE", nullptr, "BEQ",

    // 16 0xX8 instructions
    "PHP", "CLC", "PLP", "SEC", "PHA", "CLI", "PLA", "SEI", "DEY", "TYA", "TAY", "CLV", "INY",
    "CLD", "INX", "SED",

    // 16 0xXA instructions
    "ASL A", "invalid", "ROL A", "invalid", "LSR A", "invalid", "ROR A", "invalid", "TXA", "TXS",
    "TAX", "TSX", "DEX", "invalid", "NOP", "invalid"
};

Arg addr_modes[24] = {
    Arg::IndX, Arg::Zp, Arg::Imm, Arg::Abs, Arg::YInd, Arg::ZpX, Arg::AbsY, Arg::AbsX,
    Arg::Imm, Arg::Zp, Arg::Acc, Arg::Abs, Arg::Invalid, Arg::ZpX, Arg::Invalid, Arg::AbsX,
    Arg::Imm, Arg::Zp, Arg::Invalid, Arg::Abs, Arg::Invalid, Arg::ZpX, Arg::Invalid, Arg::AbsX,
};

Arg addr_modes_X0[16] = {
    Implied, Rel, Abs, Rel, Implied, Rel, Implied, Rel, Invalid, Rel, Imm, Rel, Imm, Rel, Imm, Rel
};

const char* addr_mode_to_str[] = {
    "Implied", "Rel", "Abs", "Rel", "Implied", "Rel", "Implied", "Rel", "Invalid", "Rel", "Imm",
    "Rel", "Imm", "Rel", "Imm", "Rel"
};

std::string formatted(std::string fmt, int value) {
  const char* fmt_cstr = fmt.c_str();
  char buf[64];
  std::sprintf(buf, fmt_cstr, value);
  return std::string(buf);
}

void dump_op_table(std::ostream& o) {
  auto op_table {build_ops()};
  o << "std::array<Op, 256> ops {\n";
  for (const Op& op : op_table) {
    o << "\tOp{ 0x" << hex_byte << int(op.opcode) << ", \"" << op.mnemonic << "\", "
      << addr_mode_to_str[op.addr_mode] << " },\n";
  }
  o << "};\n";
}

std::array<Op, 256> build_ops() {
  std::array<Op, 256> result;

  for (int i = 0; i < 256; ++i) {
    if (i == 0x44 || i == 0x54 || i == 0x5c || i == 0x64 || i == 0x74 || i == 0x7c || i == 0x89 ||
        i == 0x9c || i == 0x9e || i == 0xd4 || i == 0xdc || i == 0xf4 || i == 0xfc) {
      result[i] = Op {byte(i), "invalid", Invalid};
      continue;
    }

    switch (i & 0xf) {
      case 0x0: {
        auto* mnemonic = instrs[24 + ((i & 0xf0) >> 4)];
        if (mnemonic != nullptr) {
          result[i] = Op {byte(i), instrs[24 + ((i & 0xf0) >> 4)], addr_modes_X0[(i & 0xf0) >> 4]};
          continue;
        }
        break;
      }
      case 0x8:
        result[i] = Op {byte(i), instrs[40 + ((i & 0xf0) >> 4)], Implied};
        continue;
      case 0xa:
        result[i] = Op {byte(i), instrs[56 + ((i & 0xf0) >> 4)], Implied};
        continue;
      case 0x2:
        if (i == 0xa2) {
          break;
        }
      case 0x3:
      case 0x7:
      case 0xb:
        result[i] = Op {byte(i), "invalid", Invalid};
        continue;
    }

    byte cc = i & 0x3;
    byte bbb = (i & 0b00011100) >> 2;
    byte aaa = i >> 5;
    switch (cc) {
      case 0b01:
        result[i] = Op {byte(i), instrs[aaa], addr_modes[bbb]};
        continue;
      case 0b10:
        result[i] = Op {byte(i), instrs[8 + aaa], addr_modes[8 + bbb]};
        continue;
      case 0b00:
        result[i] = Op {byte(i), instrs[16 + aaa], addr_modes[16 + bbb]};
        continue;
    }

    result[i] = Op {byte(i), "invalid", Invalid};
  }

  return result;
}

std::string instruction_at_pc(CPU6502& cpu) {
  byte opcode = cpu.read(cpu.pc);
  auto meta = op_meta[opcode];

  switch (meta.addr_mode) {
    case Acc:
    case Implied:
      return meta.mnemonic;

    case Abs:
    case AbsX:
    case AbsY: {
      byte lo = cpu.read(cpu.pc + 1);
      byte hi = cpu.read(cpu.pc + 2);
      word value = (hi << 8) | lo;

      return formatted(meta.mnemonic + " " + addr_mode_templates[meta.addr_mode], value);
    }

    case Imm:
    case IndX:
    case YInd:
    case Zp:
    case ZpX: {
      byte value = cpu.read(cpu.pc + 1);
      return formatted(meta.mnemonic + " " + addr_mode_templates[meta.addr_mode], value);
    }

    case Rel: {
      byte value = cpu.read(cpu.pc + 1);
      signed char signed_value = static_cast<signed char>(value);
      word pc_plus_offset = cpu.pc + 2 + signed_value;

      return formatted(meta.mnemonic + " " + addr_mode_templates[Abs], pc_plus_offset);
    }

    case Invalid:
      break;
  }
  return meta.mnemonic + " " + addr_mode_templates[meta.addr_mode];
}
