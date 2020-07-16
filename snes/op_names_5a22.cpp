#include <map>
#include "op_names_5a22.hpp"
#include "utils.hpp"

std::array<Op, 256> op_meta {{
                                 {0x00, "BRK", Implied},
                                 {0x01, "ORA", IndX},
                                 {0x02, "COP", Implied},
                                 {0x03, "EOR", StkImm},
                                 {0x04, "TSB", Imm},
                                 {0x05, "ORA", Zp},
                                 {0x06, "ASL", Zp},
                                 {0x07, "ORA", ZpFar},
                                 {0x08, "PHP", Implied},
                                 {0x09, "ORA", Imm},
                                 {0x0a, "ASL A", Implied},
                                 {0x0b, "PHD", Implied},
                                 {0x0c, "TSB", ImmDword},
                                 {0x0d, "ORA", Abs},
                                 {0x0e, "ASL", Abs},
                                 {0x0f, "ORA", AbsDword},
                                 {0x10, "BPL", Rel},
                                 {0x11, "ORA", YInd},
                                 {0x12, "ORA", Ind},
                                 {0x13, "ORA", StkImmIndY},
                                 {0x14, "TRB", Imm},
                                 {0x15, "ORA", ZpX},
                                 {0x16, "ASL", ZpX},
                                 {0x17, "ORA", ZpFarY},
                                 {0x18, "CLC", Implied},
                                 {0x19, "ORA", AbsY},
                                 {0x1a, "INC", Acc},
                                 {0x1b, "TCS", Implied},
                                 {0x1c, "TRB", ImmDword},
                                 {0x1d, "ORA", AbsX},
                                 {0x1e, "ASL", AbsX},
                                 {0x1f, "ORA", AbsDwordX},
                                 {0x20, "JSR", Abs},
                                 {0x21, "AND", IndX},
                                 {0x22, "JSL", AbsDword},
                                 {0x23, "AND", StkImm},
                                 {0x24, "BIT", Zp},
                                 {0x25, "AND", Zp},
                                 {0x26, "ROL", Zp},
                                 {0x27, "AND", ZpFar},
                                 {0x28, "PLP", Implied},
                                 {0x29, "AND", Imm},
                                 {0x2a, "ROL A", Implied},
                                 {0x2b, "PLD", Implied},
                                 {0x2c, "BIT", Abs},
                                 {0x2d, "AND", Abs},
                                 {0x2e, "ROL", Abs},
                                 {0x2f, "AND", AbsDword},
                                 {0x30, "BMI", Rel},
                                 {0x31, "AND", YInd},
                                 {0x32, "AND", Ind},
                                 {0x33, "AND", StkImmIndY},
                                 {0x34, "BIT", ZpX},
                                 {0x35, "AND", ZpX},
                                 {0x36, "ROL", ZpX},
                                 {0x37, "AND", ZpFarY},
                                 {0x38, "SEC", Implied},
                                 {0x39, "AND", AbsY},
                                 {0x3a, "DEC", Acc},
                                 {0x3b, "TSC", Implied},
                                 {0x3c, "BIT", AbsX},
                                 {0x3d, "AND", AbsX},
                                 {0x3e, "ROL", AbsX},
                                 {0x3f, "AND", AbsDwordX},
                                 {0x40, "RTI", Implied},
                                 {0x41, "EOR", IndX},
                                 {0x42, "WDM", Implied},
                                 {0x43, "EOR", StkImm},
                                 {0x44, "MVP", BytePair},
                                 {0x45, "EOR", Zp},
                                 {0x46, "LSR", Zp},
                                 {0x47, "EOR", ZpFar},
                                 {0x48, "PHA", Implied},
                                 {0x49, "EOR", Imm},
                                 {0x4a, "LSR A", Implied},
                                 {0x4b, "PHK", Implied},
                                 {0x4c, "JMP", Abs},
                                 {0x4d, "EOR", Abs},
                                 {0x4e, "LSR", Abs},
                                 {0x4f, "EOR", AbsDword},
                                 {0x50, "BVC", Rel},
                                 {0x51, "EOR", YInd},
                                 {0x52, "EOR", Ind},
                                 {0x53, "EOR", StkImmIndY},
                                 {0x54, "MVN", BytePair},
                                 {0x55, "EOR", ZpX},
                                 {0x56, "LSR", ZpX},
                                 {0x57, "EOR", ZpFarY},
                                 {0x58, "CLI", Implied},
                                 {0x59, "EOR", AbsY},
                                 {0x5a, "PHY", Implied},
                                 {0x5b, "TCD", Implied},
                                 {0x5c, "JMP", AbsDword},
                                 {0x5d, "EOR", AbsX},
                                 {0x5e, "LSR", AbsX},
                                 {0x5f, "EOR", AbsDwordX},
                                 {0x60, "RTS", Implied},
                                 {0x61, "ADC", IndX},
                                 {0x62, "PER", Implied},
                                 {0x63, "ADC", StkImm},
                                 {0x64, "STZ", Zp},
                                 {0x65, "ADC", Zp},
                                 {0x66, "ROR", Zp},
                                 {0x67, "ADC", ZpFar},
                                 {0x68, "PLA", Implied},
                                 {0x69, "ADC", Imm},
                                 {0x6a, "ROR A", Implied},
                                 {0x6b, "RTL", Implied},
                                 {0x6c, "JMP (abs)", Abs},
                                 {0x6d, "ADC", Abs},
                                 {0x6e, "ROR", Abs},
                                 {0x6f, "ADC", AbsDword},
                                 {0x70, "BVS", Rel},
                                 {0x71, "ADC", YInd},
                                 {0x72, "ADC", Ind},
                                 {0x73, "ADC", StkImmIndY},
                                 {0x74, "STZ", ZpX},
                                 {0x75, "ADC", ZpX},
                                 {0x76, "ROR", ZpX},
                                 {0x77, "ADC", ZpFarY},
                                 {0x78, "SEI", Implied},
                                 {0x79, "ADC", AbsY},
                                 {0x7a, "PLY", Implied},
                                 {0x7b, "TDC", Implied},
                                 {0x7c, "JMP", AbsXInd},
                                 {0x7d, "ADC", AbsX},
                                 {0x7e, "ROR", AbsX},
                                 {0x7f, "ADC", AbsDwordX},
                                 {0x80, "BRA", Rel},
                                 {0x81, "STA", IndX},
                                 {0x82, "BRL", RelFar},
                                 {0x83, "STA", StkImm},
                                 {0x84, "STY", Zp},
                                 {0x85, "STA", Zp},
                                 {0x86, "STX", Zp},
                                 {0x87, "STA", ZpFar},
                                 {0x88, "DEY", Implied},
                                 {0x89, "BIT", Imm},
                                 {0x8a, "TXA", Implied},
                                 {0x8b, "PHB", Implied},
                                 {0x8c, "STY", Abs},
                                 {0x8d, "STA", Abs},
                                 {0x8e, "STX", Abs},
                                 {0x8f, "STA", AbsDword},
                                 {0x90, "BCC", Rel},
                                 {0x91, "STA", YInd},
                                 {0x92, "STA", Ind},
                                 {0x93, "STA", StkImmIndY},
                                 {0x94, "STY", ZpX},
                                 {0x95, "STA", ZpX},
                                 {0x96, "STX", ZpX},
                                 {0x97, "STA", ZpFarY},
                                 {0x98, "TYA", Implied},
                                 {0x99, "STA", AbsY},
                                 {0x9a, "TXS", Implied},
                                 {0x9b, "TXY", Implied},
                                 {0x9c, "STZ", Abs},
                                 {0x9d, "STA", AbsX},
                                 {0x9e, "STZ", AbsX},
                                 {0x9f, "STA", AbsDwordX},
                                 {0xa0, "LDY", Imm},
                                 {0xa1, "LDA", IndX},
                                 {0xa2, "LDX", Imm},
                                 {0xa3, "LDA", StkImm},
                                 {0xa4, "LDY", Zp},
                                 {0xa5, "LDA", Zp},
                                 {0xa6, "LDX", Zp},
                                 {0xa7, "LDA", ZpFar},
                                 {0xa8, "TAY", Implied},
                                 {0xa9, "LDA", Imm},
                                 {0xaa, "TAX", Implied},
                                 {0xab, "PLB", Implied},
                                 {0xac, "LDY", Abs},
                                 {0xad, "LDA", Abs},
                                 {0xae, "LDX", Abs},
                                 {0xaf, "LDA", AbsDword},
                                 {0xb0, "BCS", Rel},
                                 {0xb1, "LDA", YInd},
                                 {0xb2, "LDA", Ind},
                                 {0xb3, "LDA", StkImmIndY},
                                 {0xb4, "LDY", ZpX},
                                 {0xb5, "LDA", ZpX},
                                 {0xb6, "LDX", ZpX},
                                 {0xb7, "LDA", ZpFarY},
                                 {0xb8, "CLV", Implied},
                                 {0xb9, "LDA", AbsY},
                                 {0xba, "TSX", Implied},
                                 {0xbb, "TYX", Implied},
                                 {0xbc, "LDY", AbsX},
                                 {0xbd, "LDA", AbsX},
                                 {0xbe, "LDX", AbsX},
                                 {0xbf, "LDA", AbsDwordX},
                                 {0xc0, "CPY", Imm},
                                 {0xc1, "CMP", IndX},
                                 {0xc2, "REP", Imm},
                                 {0xc3, "CMP", StkImm},
                                 {0xc4, "CPY", Zp},
                                 {0xc5, "CMP", Zp},
                                 {0xc6, "DEC", Zp},
                                 {0xc7, "CMP", ZpFar},
                                 {0xc8, "INY", Implied},
                                 {0xc9, "CMP", Imm},
                                 {0xca, "DEX", Implied},
                                 {0xcb, "WAI", Implied},
                                 {0xcc, "CPY", Abs},
                                 {0xcd, "CMP", Abs},
                                 {0xce, "DEC", Abs},
                                 {0xcf, "CMP", AbsDword},
                                 {0xd0, "BNE", Rel},
                                 {0xd1, "CMP", YInd},
                                 {0xd2, "CMP", Ind},
                                 {0xd3, "CMP", StkImmIndY},
                                 {0xd4, "PEI", Implied},
                                 {0xd5, "CMP", ZpX},
                                 {0xd6, "DEC", ZpX},
                                 {0xd7, "CMP", ZpFarY},
                                 {0xd8, "CLD", Implied},
                                 {0xd9, "CMP", AbsY},
                                 {0xda, "PHX", Implied},
                                 {0xdb, "STP", Implied},
                                 {0xdc, "JML", AbsFar},
                                 {0xdd, "CMP", AbsX},
                                 {0xde, "DEC", AbsX},
                                 {0xdf, "CMP", AbsDwordX},
                                 {0xe0, "CPX", Imm},
                                 {0xe1, "SBC", IndX},
                                 {0xe2, "SEP", Imm},
                                 {0xe3, "SBC", StkImm},
                                 {0xe4, "CPX", Zp},
                                 {0xe5, "SBC", Zp},
                                 {0xe6, "INC", Zp},
                                 {0xe7, "SBC", ZpFar},
                                 {0xe8, "INX", Implied},
                                 {0xe9, "SBC", Imm},
                                 {0xea, "NOP", Implied},
                                 {0xeb, "XBA", Implied},
                                 {0xec, "CPX", Abs},
                                 {0xed, "SBC", Abs},
                                 {0xee, "INC", Abs},
                                 {0xef, "SBC", AbsDword},
                                 {0xf0, "BEQ", Rel},
                                 {0xf1, "SBC", YInd},
                                 {0xf2, "SBC", Ind},
                                 {0xf3, "SBC", StkImmIndY},
                                 {0xf4, "PEA", Implied},
                                 {0xf5, "SBC", ZpX},
                                 {0xf6, "INC", ZpX},
                                 {0xf7, "SBC", ZpFarY},
                                 {0xf8, "SED", Implied},
                                 {0xf9, "SBC", AbsY},
                                 {0xfa, "PLX", Implied},
                                 {0xfb, "XCE", Implied},
                                 {0xfc, "JSR", AbsXInd},
                                 {0xfd, "SBC", AbsX},
                                 {0xfe, "INC", AbsX},
                                 {0xff, "SBC", AbsDwordX},
                             }};

const char* addr_mode_templates[] = {
    "(0x00%02x + X)", "0x00%02x", "$%02x", "0x%04x", "(0x00%02x) + Y", "0x00%02x + X", "0x%04x + Y",
    "0x%04x + X", "A", "", "invalid", "+%2x", "0x%06x", "0x%06x + X",
    "[0x00%02x]", "SP + %02x", "(SP + $%02x) + Y", "+%4x", "[0x00%02x] + Y",
    "(0x%04x + X)", "$%02x, $%02x", "$%04x", "[0x00%02x]", "(0x00%02x)"
};

const char* addr_mode_to_str[] = {
    "IndX", "Zp", "Imm", "Abs", "YInd", "ZpX", "AbsY", "AbsX", "Acc", "Implied", "Invalid", "Rel"
};

std::string get_tag_for_address(word value);

std::string formatted(std::string fmt, ...) {
  va_list args;
  va_start(args, fmt);
  const char* fmt_cstr = fmt.c_str();
  char buf[64];
//  std::sprintf(buf, fmt_cstr, value);
  std::vsprintf(buf, fmt_cstr, args);
  va_end(args);
  return std::string(buf);
}

void dump_op_table(std::ostream& o) {
  auto op_table {op_meta};
  o << "{\n";
  for (const Op& op : op_table) {
    o << "\t{ 0x" << hex_byte << int(op.opcode) << ", \"" << op.mnemonic << "\", "
      << addr_mode_to_str[op.addr_mode] << " },\n";
  }
  o << "};\n";
}

std::string instruction_at_pc(CPU5A22& cpu) {
  byte opcode = cpu.read(cpu.pc.addr);
  auto meta = op_meta[opcode];

  switch (meta.addr_mode) {
    case Implied:
      return meta.mnemonic;

    case Acc:
      return meta.mnemonic + " " + addr_mode_templates[meta.addr_mode];

    case BytePair: {
      byte src = cpu.read(cpu.pc.addr + 1);
      byte dst = cpu.read(cpu.pc.addr + 2);
      return formatted(meta.mnemonic + " " + addr_mode_templates[meta.addr_mode], src, dst);
    }

    case Abs:
    case AbsX:
    case AbsY:
    case AbsFar:
    case AbsXInd: {
      byte lo = cpu.read(cpu.pc.addr + 1);
      byte hi = cpu.read(cpu.pc.addr + 2);
      word value = (hi << 8) | lo;

      std::string tag = get_tag_for_address(value);
      if (tag.size()) {
        tag = " [" + tag + "]";
      }
      return formatted(meta.mnemonic + " " + addr_mode_templates[meta.addr_mode] + tag,
                       value);
    }

    case Imm:
    case IndX:
    case YInd:
    case Zp:
    case ZpX:
    case ZpFar:
    case ZpFarY:
    case StkImm:
    case StkImmIndY:
    case Ind: {
      // TODO: need to distinguish A access from X,Y access and check m,x flags respectively
      byte value = cpu.read(cpu.pc.addr + 1);
      return formatted(meta.mnemonic + " " + addr_mode_templates[meta.addr_mode], value);
    }

    case Rel: {
      byte value = cpu.read(cpu.pc.addr + 1);
      signed char signed_value = static_cast<signed char>(value);
      word pc_plus_offset = cpu.pc.addr + 2 + signed_value;

      return formatted(meta.mnemonic + " " + addr_mode_templates[Abs], pc_plus_offset);
    }

    case RelFar: {
      word value = cpu.read_word(cpu.pc.addr + 1);
      sword signed_value = static_cast<sword>(value);
      word pc_plus_offset = cpu.pc.addr + 2 + signed_value;

      return formatted(meta.mnemonic + " " + addr_mode_templates[RelFar], pc_plus_offset);
    }

    case ImmDword: {
      byte lo = cpu.read(cpu.pc.addr + 1);
      byte hi = cpu.read(cpu.pc.addr + 2);
      return formatted(meta.mnemonic + " " + addr_mode_templates[meta.addr_mode],
                       lo | (hi << 8));
    }

    case AbsDword:
    case AbsDwordX: {
      byte lo = cpu.read(cpu.pc.addr + 1);
      byte md = cpu.read(cpu.pc.addr + 2);
      byte hi = cpu.read(cpu.pc.addr + 3);

      dword value = lo | (md << 8) | (hi << 16);
      return formatted(meta.mnemonic + " " + addr_mode_templates[meta.addr_mode], value);
    }

    case Invalid:
      break;
  }
  return meta.mnemonic + " " + addr_mode_templates[meta.addr_mode];
}

std::map<word, std::string> tags {
    {0x2100, "INIDISP"},
    {0x2101, "OBSEL"},
    {0x2102, "OAMADDL"},
    {0x2103, "OAMADDH"},
    {0x2104, "OAMDATA"},
    {0x2105, "BGMODE"},
    {0x2106, "MOSAIC"},
    {0x2107, "BG1SC"},
    {0x2108, "BG2SC"},
    {0x2109, "BG3SC"},
    {0x210A, "BG4SC"},
    {0x210B, "BG12NBA"},
    {0x210C, "BG34NBA"},
    {0x210D, "BG1HOFS"},
    {0x210E, "BG1VOFS"},
    {0x210F, "BG2HOFS"},
    {0x2110, "BG2VOFS"},
    {0x2111, "BG3HOFS"},
    {0x2112, "BG3VOFS"},
    {0x2113, "BG4HOFS"},
    {0x2114, "BG4VOFS"},
    {0x2115, "VMAIN"},
    {0x2116, "VMADDL"},
    {0x2117, "VMADDH"},
    {0x2118, "VMDATAL"},
    {0x2119, "VMDATAH"},
    {0x211A, "M7SEL"},
    {0x211B, "M7A"},
    {0x211C, "M7B"},
    {0x211D, "M7C"},
    {0x211E, "M7D"},
    {0x211F, "M7X"},
    {0x2120, "M7Y"},
    {0x2121, "CGADD"},
    {0x2122, "CGDATA"},
    {0x2123, "W12SEL"},
    {0x2124, "W34SEL"},
    {0x2125, "WOBJSEL"},
    {0x2126, "WH0"},
    {0x2127, "WH1"},
    {0x2128, "WH2"},
    {0x2129, "WH3"},
    {0x212A, "WBGLOG"},
    {0x212B, "WOBJLOG"},
    {0x212C, "TM"},
    {0x212D, "TS"},
    {0x212E, "TMW"},
    {0x212F, "TSW"},
    {0x2130, "CGWSEL"},
    {0x2131, "CGADSUB"},
    {0x2132, "COLDATA"},
    {0x2133, "SETINI"},
    {0x2134, "MPYL"},
    {0x2135, "MPYM"},
    {0x2136, "MPYH"},
    {0x2137, "SLHV"},
    {0x2138, "OAMDATAREAD"},
    {0x2139, "VMDATALREAD"},
    {0x213A, "VMDATAHREAD"},
    {0x213B, "CGDATAREAD"},
    {0x213C, "OPHCT"},
    {0x213D, "OPVCT"},
    {0x213E, "STAT77"},
    {0x213F, "STAT78"},
    {0x2140, "APUIO0"},
    {0x2141, "APUIO1"},
    {0x2142, "APUIO2"},
    {0x2143, "APUIO3"},
    {0x2180, "WMDATA"},
    {0x2181, "WMADDL"},
    {0x2182, "WMADDM"},
    {0x2183, "WMADDH"},

    //A-Bus registers (CPU registers)
    {0x4016, "JOYSER0"},
    {0x4017, "JOYSER1"},

    {0x4200, "NMITIMEN"},
    {0x4201, "WRIO"},
    {0x4202, "WRMPYA"},
    {0x4203, "WRMPYB"},
    {0x4204, "WRDIVL"},
    {0x4205, "WRDIVH"},
    {0x4206, "WRDIVB"},
    {0x4207, "HTIMEL"},
    {0x4208, "HTIMEH"},
    {0x4209, "VTIMEL"},
    {0x420A, "VTIMEH"},
    {0x420B, "MDMAEN"},
    {0x420C, "HDMAEN"},
    {0x420D, "MEMSEL"},
    {0x4210, "RDNMI"},
    {0x4211, "TIMEUP"},
    {0x4212, "HVBJOY"},
    {0x4213, "RDIO"},
    {0x4214, "RDDIVL"},
    {0x4215, "RDDIVH"},
    {0x4216, "RDMPYL"},
    {0x4217, "RDMPYH"},
    {0x4218, "JOY1L"},
    {0x4219, "JOY1H"},
    {0x421A, "JOY2L"},
    {0x421B, "JOY2H"},
    {0x421C, "JOY3L"},
    {0x421D, "JOY3H"},
    {0x421E, "JOY4L"},
    {0x421F, "JOY4H"},
    {0x4300, "DMAP0"},
    {0x4301, "BBAD0"},
    {0x4302, "A1T0L"},
    {0x4303, "A1T0H"},
    {0x4304, "A1B0"},
    {0x4305, "DAS0L"},
    {0x4306, "DAS0H"},
    {0x4307, "DAS0B"},
    {0x4308, "A2A0L"},
    {0x4309, "A2A0H"},
    {0x430a, "NTLR0"},
    {0x4310, "DMAP1"},
    {0x4311, "BBAD1"},
    {0x4312, "A1T1L"},
    {0x4313, "A1T1H"},
    {0x4314, "A1B1"},
    {0x4315, "DAS1L"},
    {0x4316, "DAS1H"},
    {0x4317, "DAS1B"},
    {0x4318, "A2A1L"},
    {0x4319, "A2A1H"},
    {0x431a, "NTLR1"},
    {0x4320, "DMAP2"},
    {0x4321, "BBAD2"},
    {0x4322, "A1T2L"},
    {0x4323, "A1T2H"},
    {0x4324, "A1B2"},
    {0x4325, "DAS2L"},
    {0x4326, "DAS2H"},
    {0x4327, "DAS2B"},
    {0x4328, "A2A2L"},
    {0x4329, "A2A2H"},
    {0x432a, "NTLR2"},
    {0x4330, "DMAP3"},
    {0x4331, "BBAD3"},
    {0x4332, "A1T3L"},
    {0x4333, "A1T3H"},
    {0x4334, "A1B3"},
    {0x4335, "DAS3L"},
    {0x4336, "DAS3H"},
    {0x4337, "DAS3B"},
    {0x4338, "A2A3L"},
    {0x4339, "A2A3H"},
    {0x433a, "NTLR3"},
    {0x4340, "DMAP4"},
    {0x4341, "BBAD4"},
    {0x4342, "A1T4L"},
    {0x4343, "A1T4H"},
    {0x4344, "A1B4"},
    {0x4345, "DAS4L"},
    {0x4346, "DAS4H"},
    {0x4347, "DAS4B"},
    {0x4348, "A2A4L"},
    {0x4349, "A2A4H"},
    {0x434a, "NTLR4"},
    {0x4350, "DMAP5"},
    {0x4351, "BBAD5"},
    {0x4352, "A1T5L"},
    {0x4353, "A1T5H"},
    {0x4354, "A1B5"},
    {0x4355, "DAS5L"},
    {0x4356, "DAS5H"},
    {0x4357, "DAS5B"},
    {0x4358, "A2A5L"},
    {0x4359, "A2A5H"},
    {0x435a, "NTLR5"},
    {0x4360, "DMAP6"},
    {0x4361, "BBAD6"},
    {0x4362, "A1T6L"},
    {0x4363, "A1T6H"},
    {0x4364, "A1B6"},
    {0x4365, "DAS6L"},
    {0x4366, "DAS6H"},
    {0x4367, "DAS6B"},
    {0x4368, "A2A6L"},
    {0x4369, "A2A6H"},
    {0x436a, "NTLR6"},
    {0x4370, "DMAP7"},
    {0x4371, "BBAD7"},
    {0x4372, "A1T7L"},
    {0x4373, "A1T7H"},
    {0x4374, "A1B7"},
    {0x4375, "DAS7L"},
    {0x4376, "DAS7H"},
    {0x4377, "DAS7B"},
    {0x4378, "A2A7L"},
    {0x4379, "A2A7H"},
    {0x437a, "NTLR7"}
};

std::string get_tag_for_address(word value) {
  return tags[value];
}
