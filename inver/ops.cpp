#include "ops.hpp"
#include "op_macros.hpp"

std::array<cycle_count_t, 256> cycle_counts {
  7,6,0,0,0,3,5,0,3,2,2,0,0,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  6,6,0,0,3,3,5,0,4,2,2,0,4,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  6,6,0,0,0,3,5,0,3,2,2,0,3,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  6,6,0,0,0,3,5,0,4,2,2,0,5,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  0,6,0,0,3,3,3,0,2,0,2,0,4,4,4,0,
  2,6,0,0,4,4,4,0,2,5,2,0,0,5,0,0,
  2,6,2,0,3,3,3,0,2,2,2,0,4,4,4,0,
  2,5,0,0,4,4,4,0,2,4,2,0,4,4,4,0,
  2,6,0,0,3,3,5,0,2,2,2,0,4,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  2,6,0,0,3,3,5,0,2,2,2,0,4,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
};

std::array<op_t, 256> ops {
  BRK,                          // 0x00
  OP(|, x_indirect),            // 0x01
  XXX,                          // 0x02
  XX(1, 8),                          // 0x03
  XX(1, 3),                          // 0x04
  OP(|, zpg),                   // 0x05
  ASL(zpg),                     // 0x06
  XX(1, 5),                          // 0x07
  PHP,                          // 0x08
  OP(|, imm),                   // 0x09
  ASL_A,                        // 0x0a
  XX(1, 2),                          // 0x0b
  XX(2, 4),                          // 0x0c
  OP(|, abs),                   // 0x0d
  ASL(abs),                     // 0x0e
  XX(2, 6),                          // 0x0f

  BRANCH((!cpu.p.N)),           // 0x10
  OP(|, indirect_y),            // 0x11
  XXX,                          // 0x12
  XX(1, 8),                          // 0x13
  XX(1, 3),                          // 0x14
  OP(|, zpg_plus_x),            // 0x15
  ASL(zpg_plus_x),              // 0x16
  XX(1, 6),                          // 0x17
  CL(C),                        // 0x18
  OP(|, abs_plus_y),            // 0x19
  XX(0, 2),                          // 0x1a
  XX(2, 7),                          // 0x1b
  XX(2, 4),                          // 0x1c TODO: cycle count can be +1
  OP(|, abs_plus_x),            // 0x1d
  ASL(abs_plus_x),              // 0x1e
  XX(2, 7),                          // 0x1f

  JSR,                          // 0x20
  OP(&, x_indirect),            // 0x21
  XXX,                          // 0x22
  XX(1, 8),                          // 0x23
  BIT(zpg),                     // 0x24
  OP(&, zpg),                   // 0x25
  ROL(zpg),                     // 0x26
  XX(1, 5),                          // 0x27
  PLP,                          // 0x28
  OP(&, imm),                   // 0x29
  ROL_A,                        // 0x2a
  XX(1, 2),                          // 0x2b
  BIT(abs),                     // 0x2c
  OP(&, abs),                   // 0x2d
  ROL(abs),                     // 0x2e
  XX(2, 6),                          // 0x2f

  BRANCH((cpu.p.N)),            // 0x30
  OP(&, indirect_y),            // 0x31
  XXX,                          // 0x32
  XX(1, 8),                          // 0x33
  XX(1, 3),                          // 0x34
  OP(&, zpg_plus_x),            // 0x35
  ROL(zpg_plus_x),              // 0x36
  XX(1, 6),                          // 0x37
  SE(C),                        // 0x38
  OP(&, abs_plus_y),            // 0x39
  XX(0, 2),                          // 0x3a
  XX(2, 7),                          // 0x3b
  XX(2, 4),                          // 0x3c TODO: +1
  OP(&, abs_plus_x),            // 0x3d
  ROL(abs_plus_x),              // 0x3e
  XX(2, 7),                          // 0x3f

  RTI,                          // 0x40
  OP(^, x_indirect),            // 0x41
  XXX,                          // 0x42
  XX(1, 8),                          // 0x43
  XX(1, 3),                          // 0x44
  OP(^, zpg),                   // 0x45
  LSR(zpg),                     // 0x46
  XX(1, 5),                          // 0x47
  PHA,                          // 0x48
  OP(^, imm),                   // 0x49
  LSR_A,                        // 0x4a
  XX(1, 2),                          // 0x4b
  JMP(abs),                     // 0x4c
  OP(^, abs),                   // 0x4d
  LSR(abs),                     // 0x4e
  XX(2, 6),                          // 0x4f

  BRANCH((!cpu.p.V)),           // 0x50
  OP(^, indirect_y),            // 0x51
  XXX,                          // 0x52
  XX(1, 8),                          // 0x53
  XX(1, 4),                          // 0x54
  OP(^, zpg_plus_x),            // 0x55
  LSR(zpg_plus_x),              // 0x56
  XX(1, 6),                          // 0x57
  CL(I),                        // 0x58
  OP(^, abs_plus_y),            // 0x59
  XX(0, 2),                          // 0x5a
  XX(2, 7),                          // 0x5b
  XX(2, 4),                          // 0x5c TODO: +1
  OP(^, abs_plus_x),            // 0x5d
  LSR(abs_plus_x),              // 0x5e
  XX(2, 7),                          // 0x5f

  RTS,                          // 0x60
  ADC(x_indirect),              // 0x61
  XXX,                          // 0x62
  XX(1, 8),                          // 0x63
  XX(1, 3),                          // 0x64
  ADC(zpg),                     // 0x65
  ROR(zpg),                     // 0x66
  XX(1, 5),                          // 0x67
  PLA,                          // 0x68
  ADC(imm),                     // 0x69
  ROR_A,                        // 0x6a
  XX(1, 2),                          // 0x6b
  JMP(indirect),                // 0x6c
  ADC(abs),                     // 0x6d
  ROR(abs),                     // 0x6e
  XX(2, 6),                          // 0x6f

  BRANCH((cpu.p.V)),            // 0x70
  ADC(indirect_y),              // 0x71
  XXX,                          // 0x72
  XX(1, 8),                          // 0x73
  XX(1, 4),                          // 0x74
  ADC(zpg_plus_x),              // 0x75
  ROR(zpg_plus_x),              // 0x76
  XX(1, 6),                          // 0x77
  SE(I),                        // 0x78
  ADC(abs_plus_y),              // 0x79
  XX(0, 2),                          // 0x7a
  XX(2, 7),                          // 0x7b
  XX(2, 4),                          // 0x7c TODO: +1
  ADC(abs_plus_x),              // 0x7d
  ROR(abs_plus_x),              // 0x7e
  XX(2, 6),                          // 0x7f

  XX(1, 2),                          // 0x80
  ST(a, x_indirect),            // 0x81
  XX(1, 2),                          // 0x82
  XX(1, 6),                          // 0x83
  ST(y, zpg),                   // 0x84
  ST(a, zpg),                   // 0x85
  ST(x, zpg),                   // 0x86
  XX(1, 3),                          // 0x87
  DE(y),                        // 0x88
  XX(1, 2),                          // 0x89
  T__(x, a),                    // 0x8a
  XX(1, 2),                          // 0x8b
  ST(y, abs),                   // 0x8c
  ST(a, abs),                   // 0x8d
  ST(x, abs),                   // 0x8e
  XX(2, 4),                          // 0x8f

  BRANCH((!cpu.p.C)),           // 0x90
  ST_NO_PAGE_CHECK(a, indirect_y), // 0x91
  XXX,                          // 0x92
  XX(1, 6),                          // 0x93
  ST(y, zpg_plus_x),            // 0x94
  ST(a, zpg_plus_x),            // 0x95
  ST(x, zpg_plus_y),            // 0x96
  XX(1, 4),                          // 0x97
  T__(y, a),                    // 0x98
  ST_NO_PAGE_CHECK(a, abs_plus_y), // 0x99
  TXS,                          // 0x9a
  XX(2, 5),                          // 0x9b
  XX(2, 5),                          // 0x9c
  ST_NO_PAGE_CHECK(a, abs_plus_x), // 0x9d
  XX(2, 5),                          // 0x9e
  XX(2, 5),                          // 0x9f

  LD(y, imm),                   // 0xa0
  LD(a, x_indirect),            // 0xa1
  LD(x, imm),                   // 0xa2
  XX(1, 6),                          // 0xa3
  LD(y, zpg),                   // 0xa4
  LD(a, zpg),                   // 0xa5
  LD(x, zpg),                   // 0xa6
  XX(1, 3),                          // 0xa7
  T__(a, y),                    // 0xa8
  LD(a, imm),                   // 0xa9
  T__(a, x),                    // 0xaa
  XX(1, 2),                          // 0xab
  LD(y, abs),                   // 0xac
  LD(a, abs),                   // 0xad
  LD(x, abs),                   // 0xae
  XX(2, 4),                          // 0xaf

  BRANCH((cpu.p.C)),            // 0xb0
  LD(a, indirect_y),            // 0xb1
  XXX,                          // 0xb2
  XX(1, 5),                          // 0xb3 TODO: +1
  LD(y, zpg_plus_x),            // 0xb4
  LD(a, zpg_plus_x),            // 0xb5
  LD(x, zpg_plus_y),            // 0xb6
  XX(1, 4),                          // 0xb7
  CL(V),                        // 0xb8
  LD(a, abs_plus_y),            // 0xb9
  T__(sp, x),                   // 0xba
  XX(2, 4),                          // 0xbb TODO: +1
  LD(y, abs_plus_x),            // 0xbc
  LD(a, abs_plus_x),            // 0xbd
  LD(x, abs_plus_y),            // 0xbe
  XX(2, 4),                          // 0xbf TODO: +1

  CP(y, imm),                   // 0xc0
  CMP(x_indirect),              // 0xc1
  XX(1, 2),                          // 0xc2
  XX(1, 8),                          // 0xc3
  CP(y, zpg),                   // 0xc4
  CMP(zpg),                     // 0xc5
  DEC(zpg),                     // 0xc6
  XX(1, 6),                          // 0xc7
  IN(y),                        // 0xc8
  CMP(imm),                     // 0xc9
  DE(x),                        // 0xca
  XX(1, 2),                          // 0xcb
  CP(y, abs),                   // 0xcc
  CMP(abs),                     // 0xcd
  DEC(abs),                     // 0xce
  XX(2, 6),                          // 0xcf

  BRANCH((!cpu.p.Z)),           // 0xd0
  CMP(indirect_y),              // 0xd1
  XXX,                          // 0xd2
  XX(1, 8),                          // 0xd3
  XX(1, 4),                          // 0xd4
  CMP(zpg_plus_x),              // 0xd5
  DEC(zpg_plus_x),              // 0xd6
  XX(1, 6),                          // 0xd7
  CL(D),                        // 0xd8
  CMP(abs_plus_y),              // 0xd9
  XX(0, 2),                          // 0xda
  XX(2, 7),                          // 0xdb
  XX(2, 4),                          // 0xdc TODO: +1
  CMP(abs_plus_x),              // 0xdd
  DEC(abs_plus_x),              // 0xde
  XX(2, 7),                          // 0xdf

  CP(x, imm),                   // 0xe0
  SBC(x_indirect),              // 0xe1
  XX(1, 2),                          // 0xe2
  XX(1, 8),                          // 0xe3
  CP(x, zpg),                   // 0xe4
  SBC(zpg),                     // 0xe5
  INC(zpg),                     // 0xe6
  XX(1, 5),                          // 0xe7
  IN(x),                        // 0xe8
  SBC(imm),                     // 0xe9
  NOP,                          // 0xea
  XX(1, 2),                          // 0xeb
  CP(x, abs),                   // 0xec
  SBC(abs),                     // 0xed
  INC(abs),                     // 0xee
  XX(2, 6),                          // 0xef

  BRANCH((cpu.p.Z)),            // 0xf0
  SBC(indirect_y),              // 0xf1
  XXX,                          // 0xf2
  XX(1, 8),                          // 0xf3
  XX(1, 4),                          // 0xf4
  SBC(zpg_plus_x),              // 0xf5
  INC(zpg_plus_x),              // 0xf6
  XX(1, 6),                          // 0xf7
  SE(D),                        // 0xf8
  SBC(abs_plus_y),              // 0xf9
  XX(0, 2),                          // 0xfa
  XX(2, 7),                          // 0xfb
  XX(2, 4),                          // 0xfc TODO: +1
  SBC(abs_plus_x),              // 0xfd
  INC(abs_plus_x),              // 0xfe
  XX(2, 7),                          // 0xff
};
