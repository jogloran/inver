#include <array>

#include "ops_5a22.hpp"
#include "op_macros_5a22.hpp"

std::array<op_record, 256> ops_65c816 {
    BRK, 7,                              // 0x00
    OP(|, x_indirect), 7,                // 0x01
    COP, 8,                              // 0x02
    OP(|, sp_plus_imm), 5,                        // 0x03
    TSB(zpg), 7,                         // 0x04
    OP(|, zpg), 4,                       // 0x05
    ASL(zpg), 7,                         // 0x06
    OP(|, zpg_far), 7,                         // 0x07
    PHP, 3,                              // 0x08
    OP(|, imm), 3,                       // 0x09
    ASL_A, 2,                            // 0x0a
    PHD, 4,                         // 0x0b
    TSB(abs), 8,                         // 0x0c
    OP(|, abs), 5,                       // 0x0d
    ASL(abs), 8,                         // 0x0e
    OP(|, abs_dword), 6,                         // 0x0f
    BRANCH((!cpu.p.N)), 2,               // 0x10
    OP(|, indirect_y), 7,                // 0x11
    OP(|, indirect), 6,                             // 0x12
    OP(|, stk_plus_imm_indirect_y), 8,                         // 0x13
    TRB(zpg), 7,                         // 0x14
    OP(|, zpg_plus_x), 5,                // 0x15
    ASL(zpg_plus_x), 8,                  // 0x16
    OP(|, zpg_far_plus_y), 7,                         // 0x17
    SE(C, 0), 2,                         // 0x18
    OP(|, abs_plus_y), 6,                // 0x19
    IN_A, 2,                         // 0x1a
    TCS, 2,                         // 0x1b
    TRB(abs), 8,                         // 0x1c
    OP(|, abs_plus_x), 6,                // 0x1d
    ASL(abs_plus_x), 9,                  // 0x1e
    OP(|, abs_dword_plus_x), 6,                         // 0x1f
    JSR, 6,                              // 0x20
    OP(&, x_indirect), 7,                // 0x21
    JSL, 5,                              // 0x22
    OP(&, sp_plus_imm), 5,                         // 0x23
    BIT(zpg), 4,                         // 0x24
    OP(&, zpg), 4,                       // 0x25
    ROL(zpg), 7,                         // 0x26
    OP(&, zpg_far), 7,                         // 0x27
    PLP, 4,                              // 0x28
    OP(&, imm), 3,                       // 0x29
    ROL_A, 2,                            // 0x2a
    PLD, 5,                         // 0x2b
    BIT(abs), 5,                         // 0x2c
    OP(&, abs), 5,                       // 0x2d
    ROL(abs), 8,                         // 0x2e
    OP(&, abs_dword), 6,                         // 0x2f
    BRANCH((cpu.p.N)), 2,                // 0x30
    OP(&, indirect_y), 7,                // 0x31
    OP(&, indirect), 6,                              // 0x32
    OP(&, stk_plus_imm_indirect_y), 8,                         // 0x33
    BIT(zpg_plus_x), 5,                         // 0x34
    OP(&, zpg_plus_x), 5,                // 0x35
    ROL(zpg_plus_x), 8,                  // 0x36
    OP(&, zpg_far_plus_y), 7,                         // 0x37
    SE(C, 1), 2,                         // 0x38
    OP(&, abs_plus_y), 6,                // 0x39
    DE_A, 2,                         // 0x3a
    TSC, 2,                         // 0x3b
    BIT(abs_plus_x), 6,                         // 0x3c
    OP(&, abs_plus_x), 6,                // 0x3d
    ROL(abs_plus_x), 9,                  // 0x3e
    OP(&, abs_dword_plus_x), 6,                         // 0x3f
    RTI, 7,                              // 0x40
    OP(^, x_indirect), 7,                // 0x41
    WDM, 2,                              // 0x42
    OP(^, sp_plus_imm), 5,                         // 0x43
    MVP, 7,                         // 0x44
    OP(^, zpg), 4,                       // 0x45
    LSR(zpg), 7,                         // 0x46
    OP(^, zpg_far), 7,                         // 0x47
    PH_(a, !cpu.p.m), 4,                              // 0x48
    OP(^, imm), 3,                       // 0x49
    LSR_A, 2,                            // 0x4a
    PHK, 3,                         // 0x4b
    JMPS(abs), 3,                         // 0x4c
    OP(^, abs), 5,                       // 0x4d
    LSR(abs), 8,                         // 0x4e
    OP(^, abs_dword), 6,                         // 0x4f
    BRANCH((!cpu.p.V)), 2,               // 0x50
    OP(^, indirect_y), 7,                // 0x51
    OP(^, indirect), 6,                              // 0x52
    OP(^, stk_plus_imm_indirect_y), 8,                         // 0x53
    MVN, 7,                         // 0x54
    OP(^, zpg_plus_x), 5,                // 0x55
    LSR(zpg_plus_x), 8,                  // 0x56
    OP(^, zpg_far_plus_y), 7,
    SE(I, 0), 2,                         // 0x58
    OP(^, abs_plus_y), 6,                // 0x59
    PH_(y, !cpu.p.x), 4,                         // 0x5a
    TCD, 2,                         // 0x5b
    JMP(abs_dword), 4,                         // 0x5c
    OP(^, abs_plus_x), 6,                // 0x5d
    LSR(abs_plus_x), 9,                  // 0x5e
    OP(^, abs_dword_plus_x), 6,                         // 0x5f
    RTS, 6,                              // 0x60
    ADC(x_indirect), 7,                  // 0x61
    PER, 6,                              // 0x62
    ADC(sp_plus_imm), 5,                         // 0x63
    STZ(zpg), 4,                         // 0x64
    ADC(zpg), 4,                         // 0x65
    ROR(zpg), 7,                         // 0x66
    ADC(zpg_far), 7,                         // 0x67
    PL_(a, !cpu.p.m), 5,                              // 0x68
    ADC(imm), 3,                         // 0x69
    ROR_A, 2,                            // 0x6a
    RTL, 6,                         // 0x6b
    JMPS(indirect), 5,                    // 0x6c
    ADC(abs), 5,                         // 0x6d
    ROR(abs), 8,                         // 0x6e
    ADC(abs_dword), 6,                         // 0x6f
    BRANCH((cpu.p.V)), 2,                // 0x70
    ADC(indirect_y), 7,                  // 0x71
    ADC(indirect), 6,                              // 0x72
    ADC(stk_plus_imm_indirect_y), 8,                         // 0x73
    STZ(zpg_plus_x), 5,                         // 0x74
    ADC(zpg_plus_x), 5,                  // 0x75
    ROR(zpg_plus_x), 8,                  // 0x76
    ADC(zpg_far_plus_y), 7,                         // 0x77
    SE(I, 1), 2,                         // 0x78
    ADC(abs_plus_y), 6,                  // 0x79
    PL_(y, !cpu.p.x), 5,                         // 0x7a
    TDC, 2,                         // 0x7b
    JMPS(abs_plus_x_indirect), 6,                         // 0x7c
    ADC(abs_plus_x), 6,                  // 0x7d
    ROR(abs_plus_x), 9,                  // 0x7e
    ADC(abs_dword_plus_x), 6,                         // 0x7f
    BRANCH((true)), 3,                         // 0x80
    STA(x_indirect), 7,                // 0x81
    BRL, 4,                         // 0x82
    STA(sp_plus_imm), 5,                         // 0x83
    ST(y, zpg), 4,                       // 0x84
    STA(zpg), 4,                       // 0x85
    ST(x, zpg), 4,                       // 0x86
    STA(zpg_far), 7,                         // 0x87
    DE(y), 2,                            // 0x88
    BIT_NUM, 3,                         // 0x89
    T_A(x), 2,                        // 0x8a
    PHB, 3,                         // 0x8b
    ST(y, abs), 5,                       // 0x8c
    STA(abs), 5,                       // 0x8d
    ST(x, abs), 5,                       // 0x8e
    STA(abs_dword), 6,                         // 0x8f
    BRANCH((!cpu.p.C)), 2,               // 0x90
    STA(indirect_y), 7,  // 0x91
    STA(indirect), 6,                              // 0x92
    STA(stk_plus_imm_indirect_y), 2,                         // 0x93
    ST(y, zpg_plus_x), 5,                // 0x94
    STA(zpg_plus_x), 5,                // 0x95
    ST(x, zpg_plus_y), 5,                // 0x96
    STA(zpg_far_plus_y), 7,                         // 0x97
    T_A(y), 2,                        // 0x98
    STA(abs_plus_y), 6,  // 0x99
    TXS, 2,                              // 0x9a
    T__(x, y), 2,                         // 0x9b
    STZ(abs), 5,                         // 0x9c
    STA(abs_plus_x), 6,  // 0x9d
    STZ(abs_plus_x), 6,                         // 0x9e
    STA(abs_dword_plus_x), 6,                         // 0x9f
    LD(y, imm), 3,                       // 0xa0
    LDA(x_indirect), 7,                // 0xa1
    LD(x, imm), 3,                       // 0xa2
    LDA(sp_plus_imm), 5,                         // 0xa3
    LD(y, zpg), 4,                       // 0xa4
    LDA(zpg), 4,                       // 0xa5
    LD(x, zpg), 4,                       // 0xa6
    LDA(zpg_far), 7,                         // 0xa7
    T__(a, y), 2,                        // 0xa8
    LDA(imm), 3,                       // 0xa9
    T__(a, x), 2,                        // 0xaa
    PLB, 4,                         // 0xab
    LD(y, abs), 5,                       // 0xac
    LDA(abs), 5,                       // 0xad
    LD(x, abs), 5,                       // 0xae
    LDA(abs_dword), 6,                         // 0xaf
    BRANCH((cpu.p.C)), 2,                // 0xb0
    LDA(indirect_y),7,                // 0xb1
    LDA(indirect), 6,                              // 0xb2
    LDA(stk_plus_imm_indirect_y), 8,                         // 0xb3
    LD(y, zpg_plus_x), 5,                // 0xb4
    LDA(zpg_plus_x), 5,                // 0xb5
    LD(x, zpg_plus_y), 5,                // 0xb6
    LDA(zpg_far_plus_y), 7,                         // 0xb7
    SE(V, 0), 2,                         // 0xb8
    LDA(abs_plus_y), 6,                // 0xb9
    T__(sp, x), 2,                       // 0xba
    T__(y, x), 2,                         // 0xbb
    LD(y, abs_plus_x), 6,                // 0xbc
    LDA(abs_plus_x), 6,                // 0xbd
    LD(x, abs_plus_y), 6,                // 0xbe
    LDA(abs_dword_plus_x), 6,                         // 0xbf
    CP(y, imm), 3,                       // 0xc0
    CMP(x_indirect), 7,                  // 0xc1
    REP, 3,                         // 0xc2
    CMP(sp_plus_imm), 5,                         // 0xc3
    CP(y, zpg), 4,                       // 0xc4
    CMP(zpg), 4,                         // 0xc5
    DEC(zpg), 7,                         // 0xc6
    CMP(zpg_far), 7,                         // 0xc7
    IN(y), 2,                            // 0xc8
    CMP(imm), 3,                         // 0xc9
    DE(x), 2,                            // 0xca
    WAI, 3,                         // 0xcb
    CP(y, abs), 5,                       // 0xcc
    CMP(abs), 5,                         // 0xcd
    DEC(abs), 7,                         // 0xce
    CMP(abs_dword), 6,                         // 0xcf
    BRANCH((!cpu.p.Z)), 2,               // 0xd0
    CMP(indirect_y), 7,                  // 0xd1
    CMP(indirect), 6,                              // 0xd2
    CMP(stk_plus_imm_indirect_y), 8,                         // 0xd3
    PEI, 6,                         // 0xd4
    CMP(zpg_plus_x), 5,                  // 0xd5
    DEC(zpg_plus_x), 8,                  // 0xd6
    CMP(zpg_far_plus_y), 7,                         // 0xd7
    SE(D, 0), 2,                         // 0xd8
    CMP(abs_plus_y), 6,                  // 0xd9
    PH_(x, !cpu.p.x), 4,                         // 0xda
    STP, 3,                         // 0xdb
    JMP(abs_far), 6,                         // 0xdc
    CMP(abs_plus_x), 6,                  // 0xdd
    DEC(abs_plus_x), 9,                  // 0xde
    CMP(abs_dword_plus_x), 6,                         // 0xdf
    CP(x, imm), 3,                       // 0xe0
    SBC(x_indirect), 7,                  // 0xe1
    SEP, 3,                         // 0xe2
    SBC(sp_plus_imm), 5,                         // 0xe3
    CP(x, zpg), 4,                       // 0xe4
    SBC(zpg), 4,                         // 0xe5
    INC(zpg), 7,                         // 0xe6
    SBC(zpg_far), 7,                         // 0xe7
    IN(x), 2,                            // 0xe8
    SBC(imm), 3,                         // 0xe9
    NOP, 2,                              // 0xea
    XBA, 3,                         // 0xeb
    CP(x, abs), 5,                       // 0xec
    SBC(abs), 5,                         // 0xed
    INC(abs), 8,                         // 0xee
    SBC(abs_dword), 6,                         // 0xef
    BRANCH((cpu.p.Z)), 2,                // 0xf0
    SBC(indirect_y), 7,                  // 0xf1
    SBC(indirect), 6,                              // 0xf2
    SBC(stk_plus_imm_indirect_y), 8,                         // 0xf3
    PEA, 5,                         // 0xf4
    SBC(zpg_plus_x), 5,                  // 0xf5
    INC(zpg_plus_x), 8,                  // 0xf6
    SBC(zpg_far_plus_y), 7,                         // 0xf7
    SE(D, 1), 2,                         // 0xf8
    SBC(abs_plus_y), 6,                  // 0xf9
    PL_(x, !cpu.p.x), 5,                         // 0xfa
    XCE, 2,                         // 0xfb
    JSR_abs_plus_x_indirect, 8,                         // 0xfc
    SBC(abs_plus_x), 6,                  // 0xfd
    INC(abs_plus_x), 9,                  // 0xfe
    SBC(abs_dword_plus_x), 6,                         // 0xff
};