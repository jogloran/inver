#pragma once

#include "types.h"
#include "mmu.h"
#include "ppu_cgb.h"
#include "ppu_dmg.h"
#include "ppu_base.h"

#include <array>
#include <iomanip>
#include <memory>
#include "flags.h"


DECLARE_string(model);

extern bool ASOBITOMO_DEBUG;

constexpr int NINSTR = 256;

constexpr long ncycles_cb[NINSTR] = {
// x0 x1 x2 x3 x4 x5  x6 x7 x8 x9 xA xB xC xD  xE xF
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 1x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 2x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 3x
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 4x
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 5x
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 6x
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 7x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 8x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 9x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Ax
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Bx
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Cx
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Dx
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Ex
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Fx
};

constexpr long ncycles[NINSTR] = {
// C9 (RET) is treated as 4 cycles so that we can reuse the conditional
// RET logic (which adds 12 for a true branch)
// same with C3 (JP a16), CD (CALL a16, treat as 12 cycles), 18 (JR r8)
// x0   x1 x2  x3  x4  x5  x6  x7  x8  x9  xA  xB  xC  xD  xE  xF
    4,  12, 8,  8,  4,  4,  8,  4,  20, 8,  8,  8,  4,  4,  8,  4, // 0x
    4,  12, 8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4, // 1x
    8,  12, 8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4, // 2x
    8,  12, 8,  8,  12, 12, 12, 4,  8,  8,  8,  8,  4,  4,  8,  4, // 3x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 4x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 5x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 6x
    8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4, // 7x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 8x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 9x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // Ax
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // Bx
    8,  12, 12, 12, 12, 16, 8,  16, 8,  4, 12,  0,  12, 12, 8,  16,// Cx
    8,  12, 12, 0,  12, 16, 8,  16, 8, 16, 12,  0,  12, 0,  8,  16,// Dx
    12, 12, 8,  0,  0,  16, 8,  16, 16, 4,  16, 0,  0,  0,  8,  16,// Ex
    12, 12, 8,  4,  0,  16, 8,  16, 12, 8,  16, 4,  0,  0,  8,  16,// Fx
};

typedef void (*op)(CPU&);

class CPU {
public:
  enum class Model {
    DMG,
    CGB,
  };
  
  std::unique_ptr<PPU> get_model(Model model) {
    switch (model) {
      case Model::DMG: return std::make_unique<GameBoyPPU>(*this);
      case Model::CGB: return std::make_unique<ColorGameBoyPPU>(*this);
      default:
        throw std::runtime_error("Invalid model type");
    }
  }
  
  Model interpret_model(std::string rom_path, std::string model_string) {
    if (model_string == "") {
      std::string extension = rom_path.substr(rom_path.find_last_of(".") + 1);
      if (extension == "gbc") {
        return Model::CGB;
      } else if (extension == "gb") {
        return Model::DMG;
      }
    }
    if (model_string == "DMG") return Model::DMG;
    if (model_string == "CGB") return Model::CGB;
    
    throw std::runtime_error("Invalid model type");
  }
  
  CPU(std::string path): CPU(path, interpret_model(path, FLAGS_model)) {}
  
  CPU(std::string path, Model model): a(0), f(0), b(0), c(0), d(0), e(0), h(0), l(0),
    pc(0x0000), sp(0x0000), cycles(0), model(model),
    timer(*this),
    ppu(get_model(model)), apu(), mmu(path, *ppu, apu, timer),
    halted(false), in_halt_bug(false), interrupt_flags_before_halt(0),
    interrupt_enabled(InterruptState::Disabled), in_cb(false),
    dbl(false), prepare_dbl(false) {
  }

  enum class InterruptState {
    Disabled,
    DisableNext,
    EnableNext,
    Enabled,
  };

  bool Z() { return (f & Zf); }
  bool N() { return (f & Nf); }
  bool H() { return (f & Hf); }
  bool C() { return (f & Cf); }

  void unset_flags(byte flags) {
    f &= ~flags;
  }

  void set_flags(byte flags) {
    f |= flags;
  }

  void toggle_flags(byte flags) {
    f ^= flags;
  }
  
  byte get_byte() {
    auto b = mmu[pc];
    pc += 1;
    return b;
  }
  
  sbyte get_sbyte() {
    auto r8 = static_cast<sbyte>(mmu[pc]);
    pc += 1;
    return r8;
  }

  word get_word() {
    auto w = (mmu[pc + 1] << 8) | mmu[pc];
    pc += 2;
    return w;
  }

  word get_word(byte hi, byte lo) {
    return (hi << 8) | lo;
  }
  
  word pop_word() {
    word result = (mmu[sp + 1] << 8) | mmu[sp + 0];
    sp += 2;
    return result;
  }
  
  void push_word(word w) {
    mmu.set(sp - 1, w >> 8);
    mmu.set(sp - 2, w & 0xff);
    sp -= 2;
  }
  
  void push_word(byte hi, byte lo) {
    mmu.set(sp - 1, hi);
    mmu.set(sp - 2, lo);
    sp -= 2;
  }
  
  bool wake_if_interrupt_requested();
  
  void halt();
  void stop();

  std::string op_name_for(word loc);
  void dump_registers_to_file(std::ofstream& out);
  void dump_state();

  void step(bool debug = false);
  void inst();
  
  void fake_boot();

  void update_interrupt_state();
  void fire_interrupts();

  void enable_interrupts() {
    interrupt_enabled = InterruptState::Enabled;
  }
  void enable_interrupts_next_instruction() {
    interrupt_enabled = InterruptState::EnableNext;
  }
  void disable_interrupts() {
    interrupt_enabled = InterruptState::Disabled;
  }

  void initiate_dma(word src);

  byte a, f, b, c, d, e, h, l;
  word pc, sp;
  long cycles;

  Model model;
  Timer timer;
  std::unique_ptr<PPU> ppu;
  APU apu;
  MMU mmu;
  
  bool halted;
  bool in_halt_bug;
  byte interrupt_flags_before_halt;

  InterruptState interrupt_enabled;
  std::string ppu_state_as_string(PPU::Mode mode);
  std::string interrupt_state_as_string(InterruptState state);
  
  bool in_cb;
  
  bool dbl;
  bool prepare_dbl;

  static constexpr size_t NINSTR = 256;

  void _handle_cb() {
    in_cb = true;
    
    byte instr = mmu[pc];
    ++pc;
    cb_ops[instr](*this);
    cycles += ncycles_cb[instr];
  }

  static void handle_cb(CPU& cpu) {
    cpu._handle_cb();
  }
  
  void check_zero(byte value) {
    if (value == 0x0) {
      set_flags(Zf);
    } else {
      unset_flags(Zf);
    }
  }
  
  void check_half_carry_word(word reg, word addend) {
    if ((reg & 0xfff) + (addend & 0xfff) > 0xfff) {
      set_flags(Hf);
    } else {
      unset_flags(Hf);
    }
  }
  
  void check_half_carry(byte reg, byte addend, byte carry=0x0) {
    if (((reg & 0xf) + (addend & 0xf) + carry) > 0xf) {
      set_flags(Hf);
    } else {
      unset_flags(Hf);
    }
  }
  
  inline void check_carry(int result) {
    if (result & (1 << 8)) {
      set_flags(Cf);
    } else {
      unset_flags(Cf);
    }
  }
  
  void check_half_carry_sub(byte reg, byte operand, byte carry=0x0) {
    if ((reg & 0xf) < ((operand & 0xf) + carry)) {
      set_flags(Hf);
    } else {
      unset_flags(Hf);
    }
  }
  
  void check_carry(word value) {
    if (value & (1 << 8)) {
      set_flags(Cf);
    } else {
      unset_flags(Cf);
    }
  }
  
  void check_carry_word(uint32_t value) {
    if (value & (1 << 16)) {
      set_flags(Cf);
    } else {
      unset_flags(Cf);
    }
  }
  
  byte check_carry_if_lobit_set(byte value) {
    byte lobit = value & 0x1;
    if (lobit) {
      set_flags(Cf);
    } else {
      unset_flags(Cf);
    }
    return lobit;
  }
  
  byte check_carry_if_hibit_set(byte value) {
    byte hibit = value >> 7;
    if (hibit) {
      set_flags(Cf);
    } else {
      unset_flags(Cf);
    }
    return hibit;
  }
  
  void speed_switch_prepare() {
    prepare_dbl = true;
    mmu.mem[0xff4d] = 1; // Set ff4d bit 0 to show the CPU is in the "prepare" state
  }

  static std::array<op, NINSTR> cb_ops;
  static std::array<op, NINSTR> ops;
  
  std::string get_pc_spec();
};
