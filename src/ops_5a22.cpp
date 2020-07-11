#include <array>

#include "ops_5a22.hpp"
#include "op_macros_5a22.hpp"

std::array<op_record, 256> ops_65c816 {
    BRK, 7,                              // 0x00
    OP(|, x_indirect), 6,                // 0x01
    COP, 0,                              // 0x02
    OP(|, sp_plus_imm), 0,                        // 0x03
    TSB(d), 0,                         // 0x04
    OP(|, zpg), 3,                       // 0x05
    ASL(zpg), 5,                         // 0x06
    OP(|, zpg_far), 0,                         // 0x07
    PHP, 3,                              // 0x08
    OP(|, imm), 2,                       // 0x09
    ASL_A, 2,                            // 0x0a
    PHD, 0,                         // 0x0b
    TSB(a), 0,                         // 0x0c
    OP(|, abs), 4,                       // 0x0d
    ASL(abs), 6,                         // 0x0e
    OP(|, al), 0,                         // 0x0f
    BRANCH((!cpu.p.N)), 2,               // 0x10
    OP(|, indirect_y), 5,                // 0x11
    OP(|, zpg_far), 0,                             // 0x12
    OP(|, stk_plus_imm_indirect_y), 0,                         // 0x13
    TRB(d), 0,                         // 0x14
    OP(|, zpg_plus_x), 4,                // 0x15
    ASL(zpg_plus_x), 6,                  // 0x16
    OP(|, zpg_far_plus_y), 0,                         // 0x17
    SE(C, 0), 2,                         // 0x18
    OP(|, abs_plus_y), 4,                // 0x19
    IN(a), 0,                         // 0x1b
    TCS, 0,                         // 0x1a
    TRB(a), 0,                         // 0x1c TODO: cycle count can be +1
    OP(|, abs_plus_x), 4,                // 0x1d
    ASL(abs_plus_x), 7,                  // 0x1e
    OP(|, al_x), 0,                         // 0x1f
    JSR, 6,                              // 0x20
    OP(&, x_indirect), 6,                // 0x21
    JSR, 0,                              // 0x22
    OP(&, sp_plus_imm), 0,                         // 0x23
    BIT(zpg), 3,                         // 0x24
    OP(&, zpg), 3,                       // 0x25
    ROL(zpg), 5,                         // 0x26
    OP(&, zpg_far), 0,                         // 0x27
    PLP, 4,                              // 0x28
    OP(&, imm), 2,                       // 0x29
    ROL_A, 2,                            // 0x2a
    PLD, 0,                         // 0x2b
    BIT(abs), 4,                         // 0x2c
    OP(&, abs), 4,                       // 0x2d
    ROL(abs), 6,                         // 0x2e
    OP(&, al), 0,                         // 0x2f
    BRANCH((cpu.p.N)), 2,                // 0x30
    OP(&, indirect_y), 5,                // 0x31
    OP(&, zpg_far), 0,                              // 0x32
    OP(&, stk_plus_imm_indirect_y), 0,                         // 0x33
    BIT(zpg_plus_x), 0,                         // 0x34
    OP(&, zpg_plus_x), 4,                // 0x35
    ROL(zpg_plus_x), 6,                  // 0x36
    OP(&, zpg_far_plus_y), 0,                         // 0x37
    SE(C, 1), 2,                         // 0x38
    OP(&, abs_plus_y), 4,                // 0x39
    DE(a), 0,                         // 0x3a
    TSC, 0,                         // 0x3b
    BIT(zpg_plus_x), 0,                         // 0x3c TODO: +1
    OP(&, abs_plus_x), 4,                // 0x3d
    ROL(abs_plus_x), 7,                  // 0x3e
    OP(&, al_x), 0,                         // 0x3f
    RTI, 6,                              // 0x40
    OP(^, x_indirect), 6,                // 0x41
    WDM, 0,                              // 0x42
    OP(^, sp_plus_imm), 0,                         // 0x43
    MVP, 0,                         // 0x44
    OP(^, zpg), 3,                       // 0x45
    LSR(zpg), 5,                         // 0x46
    OP(^, zpg_far), 0,                         // 0x47
    PHA, 3,                              // 0x48
    OP(^, imm), 2,                       // 0x49
    LSR_A, 2,                            // 0x4a
    PHK, 0,                         // 0x4b
    JMP(abs), 3,                         // 0x4c
    OP(^, abs), 4,                       // 0x4d
    LSR(abs), 6,                         // 0x4e
    OP(^, al), 0,                         // 0x4f
    BRANCH((!cpu.p.V)), 2,               // 0x50
    OP(^, indirect_y), 5,                // 0x51
    OP(^, zpg_far), 0,                              // 0x52
    OP(^, stk_plus_imm_indirect_y), 0,                         // 0x53
    STZ(d), 0,                         // 0x54
    OP(^, zpg_plus_x), 4,                // 0x55
    LSR(zpg_plus_x), 6,                  // 0x56
    OP(^, zpg_far_plus_y),0,
    SE(I, 0), 2,                         // 0x58
    OP(^, abs_plus_y), 4,                // 0x59
    PHY, 0,                         // 0x5a
    TCD, 0,                         // 0x5b
    JMP(abs_dword), 0,                         // 0x5c TODO: +1
    OP(^, abs_plus_x), 4,                // 0x5d
    LSR(abs_plus_x), 7,                  // 0x5e
    OP(^, al_x), 0,                         // 0x5f
    RTS, 6,                              // 0x60
    ADC(x_indirect), 6,                  // 0x61
    PER, 0,                              // 0x62
    ADC(sp_plus_imm), 0,                         // 0x63
    STZ(d), 0,                         // 0x64
    ADC(zpg), 3,                         // 0x65
    ROR(zpg), 5,                         // 0x66
    ADC(zpg_far), 0,                         // 0x67
    PLA, 4,                              // 0x68
    ADC(imm), 2,                         // 0x69
    ROR_A, 2,                            // 0x6a
    RTL, 0,                         // 0x6b
    JMP(indirect), 5,                    // 0x6c
    ADC(abs), 4,                         // 0x6d
    ROR(abs), 6,                         // 0x6e
    ADC(al), 0,                         // 0x6f
    BRANCH((cpu.p.V)), 2,                // 0x70
    ADC(indirect_y), 5,                  // 0x71
    ADC(zpg_far), 0,                              // 0x72
    ADC(stk_plus_imm_indirect_y), 0,                         // 0x73
    STZ(zpg_plus_x), 0,                         // 0x74
    ADC(zpg_plus_x), 4,                  // 0x75
    ROR(zpg_plus_x), 6,                  // 0x76
    ADC(zpg_far_plus_y), 0,                         // 0x77
    SE(I, 1), 2,                         // 0x78
    ADC(abs_plus_y), 4,                  // 0x79
    PLY, 0,                         // 0x7a
    TDC, 0,                         // 0x7b
    JMP(abs_plus_x_indirect), 0,                         // 0x7c TODO: +1
    ADC(abs_plus_x), 4,                  // 0x7d
    ROR(abs_plus_x), 7,                  // 0x7e
    ADC(al_x), 0,                         // 0x7f
    BRA, 0,                         // 0x80
    ST(a, x_indirect), 6,                // 0x81
    BRL, 0,                         // 0x82
    ST(a, sp_plus_imm), 0,                         // 0x83
    ST(y, zpg), 3,                       // 0x84
    ST(a, zpg), 3,                       // 0x85
    ST(x, zpg), 3,                       // 0x86
    ST(a, zpg_far), 0,                         // 0x87
    DE(y.w), 2,                            // 0x88
    BIT_NUM, 0,                         // 0x89
    T__(x, a), 2,                        // 0x8a
    PHB, 0,                         // 0x8b
    ST(y, abs), 4,                       // 0x8c
    ST(a, abs), 4,                       // 0x8d
    ST(x, abs), 4,                       // 0x8e
    ST(a, abs_dword), 0,                         // 0x8f
    BRANCH((!cpu.p.C)), 2,               // 0x90
    ST_NO_PAGE_CHECK(a, indirect_y), 6,  // 0x91
    ST(a, zpg_far), 0,                              // 0x92
    ST(a, stk_plus_imm_indirect_y), 0,                         // 0x93
    ST(y, zpg_plus_x), 4,                // 0x94
    ST(a, zpg_plus_x), 4,                // 0x95
    ST(x, zpg_plus_y), 4,                // 0x96
    ST(a, zpg_far_plus_y), 0,                         // 0x97
    T__(y, a), 2,                        // 0x98
    ST_NO_PAGE_CHECK(a, abs_plus_y), 5,  // 0x99
    TXS, 2,                              // 0x9a
    TXY, 0,                         // 0x9b
    STZ(a), 0,                         // 0x9c
    ST_NO_PAGE_CHECK(a, abs_plus_x), 5,  // 0x9d
    STZ(a_X), 0,                         // 0x9e
    ST(a, abs_dword_plus_x), 0,                         // 0x9f
    LD(y, imm), 2,                       // 0xa0
    LD(a, x_indirect), 6,                // 0xa1
    LD(x, imm), 2,                       // 0xa2
    LD(a, sp_plus_imm), 0,                         // 0xa3
    LD(y, zpg), 3,                       // 0xa4
    LD(a, zpg), 3,                       // 0xa5
    LD(x, zpg), 3,                       // 0xa6
    LD(a, zpg_far), 0,                         // 0xa7
    T__(a, y), 2,                        // 0xa8
    LD(a, imm), 2,                       // 0xa9
    T__(a, x), 2,                        // 0xaa
    PLB, 0,                         // 0xab
    LD(y, abs), 4,                       // 0xac
    LD(a, abs), 4,                       // 0xad
    LD(x, abs), 4,                       // 0xae
    LD(a, al), 0,                         // 0xaf
    BRANCH((cpu.p.C)), 2,                // 0xb0
    LD(a, indirect_y), 5,                // 0xb1
    LD(a, zpg_far), 0,                              // 0xb2
    LD(a, stk_plus_imm_indirect_y), 0,                         // 0xb3 TODO: +1
    LD(y, zpg_plus_x), 4,                // 0xb4
    LD(a, zpg_plus_x), 4,                // 0xb5
    LD(x, zpg_plus_y), 4,                // 0xb6
    LD(a, zpg_far_plus_y), 0,                         // 0xb7
    SE(V, 0), 2,                         // 0xb8
    LD(a, abs_plus_y), 4,                // 0xb9
    T__(sp, x), 2,                       // 0xba
    T__(y, x), 0,                         // 0xbb TODO: +1
    LD(y, abs_plus_x), 4,                // 0xbc
    LD(a, abs_plus_x), 4,                // 0xbd
    LD(x, abs_plus_y), 4,                // 0xbe
    LD(a, al_x), 0,                         // 0xbf TODO: +1
    CP(y, imm), 2,                       // 0xc0
    CMP(x_indirect), 6,                  // 0xc1
    REP_N, 0,                         // 0xc2
    CMP(sp_plus_imm), 0,                         // 0xc3
    CP(y, zpg), 3,                       // 0xc4
    CMP(zpg), 3,                         // 0xc5
    DEC(zpg), 5,                         // 0xc6
    CMP(zpg_far), 0,                         // 0xc7
    IN(y.w), 2,                            // 0xc8
    CMP(imm), 2,                         // 0xc9
    DE(x.w), 2,                            // 0xca
    WAI, 0,                         // 0xcb
    CP(y, abs), 4,                       // 0xcc
    CMP(abs), 4,                         // 0xcd
    DEC(abs), 6,                         // 0xce
    CMP(al), 0,                         // 0xcf
    BRANCH((!cpu.p.Z)), 2,               // 0xd0
    CMP(indirect_y), 5,                  // 0xd1
    CMP(zpg_far), 0,                              // 0xd2
    CMP(stk_plus_imm_indirect_y), 0,                         // 0xd3
    PEI, 0,                         // 0xd4
    CMP(zpg_plus_x), 4,                  // 0xd5
    DEC(zpg_plus_x), 6,                  // 0xd6
    CMP(zpg_far_plus_y), 0,                         // 0xd7
    SE(D, 0), 2,                         // 0xd8
    CMP(abs_plus_y), 4,                  // 0xd9
    PHX, 0,                         // 0xda
    STP, 0,                         // 0xdb
    JMP(indirect_far), 0,                         // 0xdc TODO: +1
    CMP(abs_plus_x), 4,                  // 0xdd
    DEC(abs_plus_x), 7,                  // 0xde
    CMP(al_x), 0,                         // 0xdf
    CP(x, imm), 2,                       // 0xe0
    SBC(x_indirect), 6,                  // 0xe1
    SEP_N, 0,                         // 0xe2
    SBC(sp_plus_imm), 0,                         // 0xe3
    CP(x, zpg), 3,                       // 0xe4
    SBC(zpg), 3,                         // 0xe5
    INC(zpg), 5,                         // 0xe6
    SBC(zpg_far), 0,                         // 0xe7
    IN(x.w), 2,                            // 0xe8
    SBC(imm), 2,                         // 0xe9
    NOP, 2,                              // 0xea
    XBA, 0,                         // 0xeb
    CP(x, abs), 4,                       // 0xec
    SBC(abs), 4,                         // 0xed
    INC(abs), 6,                         // 0xee
    SBC(al), 0,                         // 0xef
    BRANCH((cpu.p.Z)), 2,                // 0xf0
    SBC(indirect_y), 5,                  // 0xf1
    SBC(zpg_far), 0,                              // 0xf2
    SBC(stk_plus_imm_indirect_y), 0,                         // 0xf3
    PEA, 0,                         // 0xf4
    SBC(zpg_plus_x), 4,                  // 0xf5
    INC(zpg_plus_x), 6,                  // 0xf6
    SBC(zpg_far_plus_y), 0,                         // 0xf7
    SE(D, 1), 2,                         // 0xf8
    SBC(abs_plus_y), 4,                  // 0xf9
    PLX, 0,                         // 0xfa
    XCE, 0,                         // 0xfb
    JSR_abs_plus_x_indirect, 0,                         // 0xfc TODO: +1
    SBC(abs_plus_x), 4,                  // 0xfd
    INC(abs_plus_x), 7,                  // 0xfe
    SBC(al_x), 0,                         // 0xff
};