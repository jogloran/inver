#include "op_names.hpp"

std::array<Op, 256> op_meta {{
                                 {0x00, "BRK", Implied},
                                 {0x01, "ORA", IndX},
                                 {0x02, "invalid", Invalid},
                                 {0x03, "invalid", Invalid},
                                 {0x04, "invalid", Zp},
                                 {0x05, "ORA", Zp},
                                 {0x06, "ASL", Zp},
                                 {0x07, "invalid", Invalid},
                                 {0x08, "PHP", Implied},
                                 {0x09, "ORA", Imm},
                                 {0x0a, "ASL A", Implied},
                                 {0x0b, "invalid", Invalid},
                                 {0x0c, "invalid", Abs},
                                 {0x0d, "ORA", Abs},
                                 {0x0e, "ASL", Abs},
                                 {0x0f, "invalid", Invalid},
                                 {0x10, "BPL", Rel},
                                 {0x11, "ORA", YInd},
                                 {0x12, "invalid", Invalid},
                                 {0x13, "invalid", Invalid},
                                 {0x14, "invalid", ZpX},
                                 {0x15, "ORA", ZpX},
                                 {0x16, "ASL", ZpX},
                                 {0x17, "invalid", Invalid},
                                 {0x18, "CLC", Implied},
                                 {0x19, "ORA", AbsY},
                                 {0x1a, "invalid", Implied},
                                 {0x1b, "invalid", Invalid},
                                 {0x1c, "invalid", AbsX},
                                 {0x1d, "ORA", AbsX},
                                 {0x1e, "ASL", AbsX},
                                 {0x1f, "invalid", Invalid},
                                 {0x20, "JSR", Abs},
                                 {0x21, "AND", IndX},
                                 {0x22, "invalid", Invalid},
                                 {0x23, "invalid", Invalid},
                                 {0x24, "BIT", Zp},
                                 {0x25, "AND", Zp},
                                 {0x26, "ROL", Zp},
                                 {0x27, "invalid", Invalid},
                                 {0x28, "PLP", Implied},
                                 {0x29, "AND", Imm},
                                 {0x2a, "ROL A", Implied},
                                 {0x2b, "invalid", Invalid},
                                 {0x2c, "BIT", Abs},
                                 {0x2d, "AND", Abs},
                                 {0x2e, "ROL", Abs},
                                 {0x2f, "invalid", Invalid},
                                 {0x30, "BMI", Rel},
                                 {0x31, "AND", YInd},
                                 {0x32, "invalid", Invalid},
                                 {0x33, "invalid", Invalid},
                                 {0x34, "BIT", ZpX},
                                 {0x35, "AND", ZpX},
                                 {0x36, "ROL", ZpX},
                                 {0x37, "invalid", Invalid},
                                 {0x38, "SEC", Implied},
                                 {0x39, "AND", AbsY},
                                 {0x3a, "invalid", Implied},
                                 {0x3b, "invalid", Invalid},
                                 {0x3c, "BIT", AbsX},
                                 {0x3d, "AND", AbsX},
                                 {0x3e, "ROL", AbsX},
                                 {0x3f, "invalid", Invalid},
                                 {0x40, "RTI", Implied},
                                 {0x41, "EOR", IndX},
                                 {0x42, "invalid", Invalid},
                                 {0x43, "invalid", Invalid},
                                 {0x44, "invalid", Invalid},
                                 {0x45, "EOR", Zp},
                                 {0x46, "LSR", Zp},
                                 {0x47, "invalid", Invalid},
                                 {0x48, "PHA", Implied},
                                 {0x49, "EOR", Imm},
                                 {0x4a, "LSR A", Implied},
                                 {0x4b, "invalid", Invalid},
                                 {0x4c, "JMP", Abs},
                                 {0x4d, "EOR", Abs},
                                 {0x4e, "LSR", Abs},
                                 {0x4f, "invalid", Invalid},
                                 {0x50, "BVC", Rel},
                                 {0x51, "EOR", YInd},
                                 {0x52, "invalid", Invalid},
                                 {0x53, "invalid", Invalid},
                                 {0x54, "invalid", Invalid},
                                 {0x55, "EOR", ZpX},
                                 {0x56, "LSR", ZpX},
                                 {0x57, "invalid", Invalid},
                                 {0x58, "CLI", Implied},
                                 {0x59, "EOR", AbsY},
                                 {0x5a, "invalid", Implied},
                                 {0x5b, "invalid", Invalid},
                                 {0x5c, "invalid", Invalid},
                                 {0x5d, "EOR", AbsX},
                                 {0x5e, "LSR", AbsX},
                                 {0x5f, "invalid", Invalid},
                                 {0x60, "RTS", Implied},
                                 {0x61, "ADC", IndX},
                                 {0x62, "invalid", Invalid},
                                 {0x63, "invalid", Invalid},
                                 {0x64, "invalid", Invalid},
                                 {0x65, "ADC", Zp},
                                 {0x66, "ROR", Zp},
                                 {0x67, "invalid", Invalid},
                                 {0x68, "PLA", Implied},
                                 {0x69, "ADC", Imm},
                                 {0x6a, "ROR A", Implied},
                                 {0x6b, "invalid", Invalid},
                                 {0x6c, "JMP (abs)", Abs},
                                 {0x6d, "ADC", Abs},
                                 {0x6e, "ROR", Abs},
                                 {0x6f, "invalid", Invalid},
                                 {0x70, "BVS", Rel},
                                 {0x71, "ADC", YInd},
                                 {0x72, "invalid", Invalid},
                                 {0x73, "invalid", Invalid},
                                 {0x74, "invalid", Invalid},
                                 {0x75, "ADC", ZpX},
                                 {0x76, "ROR", ZpX},
                                 {0x77, "invalid", Invalid},
                                 {0x78, "SEI", Implied},
                                 {0x79, "ADC", AbsY},
                                 {0x7a, "invalid", Implied},
                                 {0x7b, "invalid", Invalid},
                                 {0x7c, "invalid", Invalid},
                                 {0x7d, "ADC", AbsX},
                                 {0x7e, "ROR", AbsX},
                                 {0x7f, "invalid", Invalid},
                                 {0x80, "invalid", Invalid},
                                 {0x81, "STA", IndX},
                                 {0x82, "invalid", Invalid},
                                 {0x83, "invalid", Invalid},
                                 {0x84, "STY", Zp},
                                 {0x85, "STA", Zp},
                                 {0x86, "STX", Zp},
                                 {0x87, "invalid", Invalid},
                                 {0x88, "DEY", Implied},
                                 {0x89, "invalid", Invalid},
                                 {0x8a, "TXA", Implied},
                                 {0x8b, "invalid", Invalid},
                                 {0x8c, "STY", Abs},
                                 {0x8d, "STA", Abs},
                                 {0x8e, "STX", Abs},
                                 {0x8f, "invalid", Invalid},
                                 {0x90, "BCC", Rel},
                                 {0x91, "STA", YInd},
                                 {0x92, "invalid", Invalid},
                                 {0x93, "invalid", Invalid},
                                 {0x94, "STY", ZpX},
                                 {0x95, "STA", ZpX},
                                 {0x96, "STX", ZpX},
                                 {0x97, "invalid", Invalid},
                                 {0x98, "TYA", Implied},
                                 {0x99, "STA", AbsY},
                                 {0x9a, "TXS", Implied},
                                 {0x9b, "invalid", Invalid},
                                 {0x9c, "invalid", Invalid},
                                 {0x9d, "STA", AbsX},
                                 {0x9e, "invalid", Invalid},
                                 {0x9f, "invalid", Invalid},
                                 {0xa0, "LDY", Imm},
                                 {0xa1, "LDA", IndX},
                                 {0xa2, "LDX", Imm},
                                 {0xa3, "invalid", Invalid},
                                 {0xa4, "LDY", Zp},
                                 {0xa5, "LDA", Zp},
                                 {0xa6, "LDX", Zp},
                                 {0xa7, "invalid", Invalid},
                                 {0xa8, "TAY", Implied},
                                 {0xa9, "LDA", Imm},
                                 {0xaa, "TAX", Implied},
                                 {0xab, "invalid", Invalid},
                                 {0xac, "LDY", Abs},
                                 {0xad, "LDA", Abs},
                                 {0xae, "LDX", Abs},
                                 {0xaf, "invalid", Invalid},
                                 {0xb0, "BCS", Rel},
                                 {0xb1, "LDA", YInd},
                                 {0xb2, "invalid", Invalid},
                                 {0xb3, "invalid", Invalid},
                                 {0xb4, "LDY", ZpX},
                                 {0xb5, "LDA", ZpX},
                                 {0xb6, "LDX", ZpX},
                                 {0xb7, "invalid", Invalid},
                                 {0xb8, "CLV", Implied},
                                 {0xb9, "LDA", AbsY},
                                 {0xba, "TSX", Implied},
                                 {0xbb, "invalid", Invalid},
                                 {0xbc, "LDY", AbsX},
                                 {0xbd, "LDA", AbsX},
                                 {0xbe, "LDX", AbsX},
                                 {0xbf, "invalid", Invalid},
                                 {0xc0, "CPY", Imm},
                                 {0xc1, "CMP", IndX},
                                 {0xc2, "invalid", Invalid},
                                 {0xc3, "invalid", Invalid},
                                 {0xc4, "CPY", Zp},
                                 {0xc5, "CMP", Zp},
                                 {0xc6, "DEC", Zp},
                                 {0xc7, "invalid", Invalid},
                                 {0xc8, "INY", Implied},
                                 {0xc9, "CMP", Imm},
                                 {0xca, "DEX", Implied},
                                 {0xcb, "invalid", Invalid},
                                 {0xcc, "CPY", Abs},
                                 {0xcd, "CMP", Abs},
                                 {0xce, "DEC", Abs},
                                 {0xcf, "invalid", Invalid},
                                 {0xd0, "BNE", Rel},
                                 {0xd1, "CMP", YInd},
                                 {0xd2, "invalid", Invalid},
                                 {0xd3, "invalid", Invalid},
                                 {0xd4, "invalid", Invalid},
                                 {0xd5, "CMP", ZpX},
                                 {0xd6, "DEC", ZpX},
                                 {0xd7, "invalid", Invalid},
                                 {0xd8, "CLD", Implied},
                                 {0xd9, "CMP", AbsY},
                                 {0xda, "invalid", Implied},
                                 {0xdb, "invalid", Invalid},
                                 {0xdc, "invalid", Invalid},
                                 {0xdd, "CMP", AbsX},
                                 {0xde, "DEC", AbsX},
                                 {0xdf, "invalid", Invalid},
                                 {0xe0, "CPX", Imm},
                                 {0xe1, "SBC", IndX},
                                 {0xe2, "invalid", Invalid},
                                 {0xe3, "invalid", Invalid},
                                 {0xe4, "CPX", Zp},
                                 {0xe5, "SBC", Zp},
                                 {0xe6, "INC", Zp},
                                 {0xe7, "invalid", Invalid},
                                 {0xe8, "INX", Implied},
                                 {0xe9, "SBC", Imm},
                                 {0xea, "NOP", Implied},
                                 {0xeb, "invalid", Invalid},
                                 {0xec, "CPX", Abs},
                                 {0xed, "SBC", Abs},
                                 {0xee, "INC", Abs},
                                 {0xef, "invalid", Invalid},
                                 {0xf0, "BEQ", Rel},
                                 {0xf1, "SBC", YInd},
                                 {0xf2, "invalid", Invalid},
                                 {0xf3, "invalid", Invalid},
                                 {0xf4, "invalid", Invalid},
                                 {0xf5, "SBC", ZpX},
                                 {0xf6, "INC", ZpX},
                                 {0xf7, "invalid", Invalid},
                                 {0xf8, "SED", Implied},
                                 {0xf9, "SBC", AbsY},
                                 {0xfa, "invalid", Implied},
                                 {0xfb, "invalid", Invalid},
                                 {0xfc, "invalid", Invalid},
                                 {0xfd, "SBC", AbsX},
                                 {0xfe, "INC", AbsX},
                                 {0xff, "invalid", Invalid},
                             }};

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
    "IndX", "Zp", "Imm", "Abs", "YInd", "ZpX", "AbsY", "AbsX", "Acc", "Implied", "Invalid", "Rel"
};

std::string formatted(std::string fmt, int value) {
  const char* fmt_cstr = fmt.c_str();
  char buf[64];
  std::sprintf(buf, fmt_cstr, value);
  return std::string(buf);
}

void dump_op_table(std::ostream& o) {
  auto op_table {build_ops()};
  o << "{\n";
  for (const Op& op : op_table) {
    o << "\t{ 0x" << hex_byte << int(op.opcode) << ", \"" << op.mnemonic << "\", "
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
