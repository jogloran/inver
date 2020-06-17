#include "op_names.hpp"

std::array<Op, 256> op_meta {
    Op {0x00, "BRK", Rel},
    Op {0x01, "ORA", Implied},
    Op {0x02, "invalid", Imm},
    Op {0x03, "invalid", Imm},
    Op {0x04, "invalid", Rel},
    Op {0x05, "ORA", Rel},
    Op {0x06, "ASL", Rel},
    Op {0x07, "invalid", Imm},
    Op {0x08, "PHP", Rel},
    Op {0x09, "ORA", Abs},
    Op {0x0a, "ASL A", Rel},
    Op {0x0b, "invalid", Imm},
    Op {0x0c, "invalid", Rel},
    Op {0x0d, "ORA", Rel},
    Op {0x0e, "ASL", Rel},
    Op {0x0f, "invalid", Imm},
    Op {0x10, "BPL", Rel},
    Op {0x11, "ORA", Implied},
    Op {0x12, "invalid", Imm},
    Op {0x13, "invalid", Imm},
    Op {0x14, "invalid", Rel},
    Op {0x15, "ORA", Rel},
    Op {0x16, "ASL", Rel},
    Op {0x17, "invalid", Imm},
    Op {0x18, "CLC", Rel},
    Op {0x19, "ORA", Implied},
    Op {0x1a, "invalid", Rel},
    Op {0x1b, "invalid", Imm},
    Op {0x1c, "invalid", Rel},
    Op {0x1d, "ORA", Rel},
    Op {0x1e, "ASL", Rel},
    Op {0x1f, "invalid", Imm},
    Op {0x20, "JSR", Rel},
    Op {0x21, "AND", Implied},
    Op {0x22, "invalid", Imm},
    Op {0x23, "invalid", Imm},
    Op {0x24, "BIT", Rel},
    Op {0x25, "AND", Rel},
    Op {0x26, "ROL", Rel},
    Op {0x27, "invalid", Imm},
    Op {0x28, "PLP", Rel},
    Op {0x29, "AND", Abs},
    Op {0x2a, "ROL A", Rel},
    Op {0x2b, "invalid", Imm},
    Op {0x2c, "BIT", Rel},
    Op {0x2d, "AND", Rel},
    Op {0x2e, "ROL", Rel},
    Op {0x2f, "invalid", Imm},
    Op {0x30, "BMI", Rel},
    Op {0x31, "AND", Implied},
    Op {0x32, "invalid", Imm},
    Op {0x33, "invalid", Imm},
    Op {0x34, "BIT", Rel},
    Op {0x35, "AND", Rel},
    Op {0x36, "ROL", Rel},
    Op {0x37, "invalid", Imm},
    Op {0x38, "SEC", Rel},
    Op {0x39, "AND", Implied},
    Op {0x3a, "invalid", Rel},
    Op {0x3b, "invalid", Imm},
    Op {0x3c, "BIT", Rel},
    Op {0x3d, "AND", Rel},
    Op {0x3e, "ROL", Rel},
    Op {0x3f, "invalid", Imm},
    Op {0x40, "RTI", Rel},
    Op {0x41, "EOR", Implied},
    Op {0x42, "invalid", Imm},
    Op {0x43, "invalid", Imm},
    Op {0x44, "invalid", Imm},
    Op {0x45, "EOR", Rel},
    Op {0x46, "LSR", Rel},
    Op {0x47, "invalid", Imm},
    Op {0x48, "PHA", Rel},
    Op {0x49, "EOR", Abs},
    Op {0x4a, "LSR A", Rel},
    Op {0x4b, "invalid", Imm},
    Op {0x4c, "JMP", Rel},
    Op {0x4d, "EOR", Rel},
    Op {0x4e, "LSR", Rel},
    Op {0x4f, "invalid", Imm},
    Op {0x50, "BVC", Rel},
    Op {0x51, "EOR", Implied},
    Op {0x52, "invalid", Imm},
    Op {0x53, "invalid", Imm},
    Op {0x54, "invalid", Imm},
    Op {0x55, "EOR", Rel},
    Op {0x56, "LSR", Rel},
    Op {0x57, "invalid", Imm},
    Op {0x58, "CLI", Rel},
    Op {0x59, "EOR", Implied},
    Op {0x5a, "invalid", Rel},
    Op {0x5b, "invalid", Imm},
    Op {0x5c, "invalid", Imm},
    Op {0x5d, "EOR", Rel},
    Op {0x5e, "LSR", Rel},
    Op {0x5f, "invalid", Imm},
    Op {0x60, "RTS", Rel},
    Op {0x61, "ADC", Implied},
    Op {0x62, "invalid", Imm},
    Op {0x63, "invalid", Imm},
    Op {0x64, "invalid", Imm},
    Op {0x65, "ADC", Rel},
    Op {0x66, "ROR", Rel},
    Op {0x67, "invalid", Imm},
    Op {0x68, "PLA", Rel},
    Op {0x69, "ADC", Abs},
    Op {0x6a, "ROR A", Rel},
    Op {0x6b, "invalid", Imm},
    Op {0x6c, "JMP (abs)", Rel},
    Op {0x6d, "ADC", Rel},
    Op {0x6e, "ROR", Rel},
    Op {0x6f, "invalid", Imm},
    Op {0x70, "BVS", Rel},
    Op {0x71, "ADC", Implied},
    Op {0x72, "invalid", Imm},
    Op {0x73, "invalid", Imm},
    Op {0x74, "invalid", Imm},
    Op {0x75, "ADC", Rel},
    Op {0x76, "ROR", Rel},
    Op {0x77, "invalid", Imm},
    Op {0x78, "SEI", Rel},
    Op {0x79, "ADC", Implied},
    Op {0x7a, "invalid", Rel},
    Op {0x7b, "invalid", Imm},
    Op {0x7c, "invalid", Imm},
    Op {0x7d, "ADC", Rel},
    Op {0x7e, "ROR", Rel},
    Op {0x7f, "invalid", Imm},
    Op {0x80, "invalid", Imm},
    Op {0x81, "STA", Implied},
    Op {0x82, "invalid", Imm},
    Op {0x83, "invalid", Imm},
    Op {0x84, "STY", Rel},
    Op {0x85, "STA", Rel},
    Op {0x86, "STX", Rel},
    Op {0x87, "invalid", Imm},
    Op {0x88, "DEY", Rel},
    Op {0x89, "invalid", Imm},
    Op {0x8a, "TXA", Rel},
    Op {0x8b, "invalid", Imm},
    Op {0x8c, "STY", Rel},
    Op {0x8d, "STA", Rel},
    Op {0x8e, "STX", Rel},
    Op {0x8f, "invalid", Imm},
    Op {0x90, "BCC", Rel},
    Op {0x91, "STA", Implied},
    Op {0x92, "invalid", Imm},
    Op {0x93, "invalid", Imm},
    Op {0x94, "STY", Rel},
    Op {0x95, "STA", Rel},
    Op {0x96, "STX", Rel},
    Op {0x97, "invalid", Imm},
    Op {0x98, "TYA", Rel},
    Op {0x99, "STA", Implied},
    Op {0x9a, "TXS", Rel},
    Op {0x9b, "invalid", Imm},
    Op {0x9c, "invalid", Imm},
    Op {0x9d, "STA", Rel},
    Op {0x9e, "invalid", Imm},
    Op {0x9f, "invalid", Imm},
    Op {0xa0, "LDY", Abs},
    Op {0xa1, "LDA", Implied},
    Op {0xa2, "LDX", Abs},
    Op {0xa3, "invalid", Imm},
    Op {0xa4, "LDY", Rel},
    Op {0xa5, "LDA", Rel},
    Op {0xa6, "LDX", Rel},
    Op {0xa7, "invalid", Imm},
    Op {0xa8, "TAY", Rel},
    Op {0xa9, "LDA", Abs},
    Op {0xaa, "TAX", Rel},
    Op {0xab, "invalid", Imm},
    Op {0xac, "LDY", Rel},
    Op {0xad, "LDA", Rel},
    Op {0xae, "LDX", Rel},
    Op {0xaf, "invalid", Imm},
    Op {0xb0, "BCS", Rel},
    Op {0xb1, "LDA", Implied},
    Op {0xb2, "invalid", Imm},
    Op {0xb3, "invalid", Imm},
    Op {0xb4, "LDY", Rel},
    Op {0xb5, "LDA", Rel},
    Op {0xb6, "LDX", Rel},
    Op {0xb7, "invalid", Imm},
    Op {0xb8, "CLV", Rel},
    Op {0xb9, "LDA", Implied},
    Op {0xba, "TSX", Rel},
    Op {0xbb, "invalid", Imm},
    Op {0xbc, "LDY", Rel},
    Op {0xbd, "LDA", Rel},
    Op {0xbe, "LDX", Rel},
    Op {0xbf, "invalid", Imm},
    Op {0xc0, "CPY", Abs},
    Op {0xc1, "CMP", Implied},
    Op {0xc2, "invalid", Imm},
    Op {0xc3, "invalid", Imm},
    Op {0xc4, "CPY", Rel},
    Op {0xc5, "CMP", Rel},
    Op {0xc6, "DEC", Rel},
    Op {0xc7, "invalid", Imm},
    Op {0xc8, "INY", Rel},
    Op {0xc9, "CMP", Abs},
    Op {0xca, "DEX", Rel},
    Op {0xcb, "invalid", Imm},
    Op {0xcc, "CPY", Rel},
    Op {0xcd, "CMP", Rel},
    Op {0xce, "DEC", Rel},
    Op {0xcf, "invalid", Imm},
    Op {0xd0, "BNE", Rel},
    Op {0xd1, "CMP", Implied},
    Op {0xd2, "invalid", Imm},
    Op {0xd3, "invalid", Imm},
    Op {0xd4, "invalid", Imm},
    Op {0xd5, "CMP", Rel},
    Op {0xd6, "DEC", Rel},
    Op {0xd7, "invalid", Imm},
    Op {0xd8, "CLD", Rel},
    Op {0xd9, "CMP", Implied},
    Op {0xda, "invalid", Rel},
    Op {0xdb, "invalid", Imm},
    Op {0xdc, "invalid", Imm},
    Op {0xdd, "CMP", Rel},
    Op {0xde, "DEC", Rel},
    Op {0xdf, "invalid", Imm},
    Op {0xe0, "CPX", Abs},
    Op {0xe1, "SBC", Implied},
    Op {0xe2, "invalid", Imm},
    Op {0xe3, "invalid", Imm},
    Op {0xe4, "CPX", Rel},
    Op {0xe5, "SBC", Rel},
    Op {0xe6, "INC", Rel},
    Op {0xe7, "invalid", Imm},
    Op {0xe8, "INX", Rel},
    Op {0xe9, "SBC", Abs},
    Op {0xea, "NOP", Rel},
    Op {0xeb, "invalid", Imm},
    Op {0xec, "CPX", Rel},
    Op {0xed, "SBC", Rel},
    Op {0xee, "INC", Rel},
    Op {0xef, "invalid", Imm},
    Op {0xf0, "BEQ", Rel},
    Op {0xf1, "SBC", Implied},
    Op {0xf2, "invalid", Imm},
    Op {0xf3, "invalid", Imm},
    Op {0xf4, "invalid", Imm},
    Op {0xf5, "SBC", Rel},
    Op {0xf6, "INC", Rel},
    Op {0xf7, "invalid", Imm},
    Op {0xf8, "SED", Rel},
    Op {0xf9, "SBC", Implied},
    Op {0xfa, "invalid", Rel},
    Op {0xfb, "invalid", Imm},
    Op {0xfc, "invalid", Imm},
    Op {0xfd, "SBC", Rel},
    Op {0xfe, "INC", Rel},
    Op {0xff, "invalid", Imm},
};

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
