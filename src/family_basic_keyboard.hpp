#pragma once

#include <SDL2/SDL.h>

#include "types.h"
#include "peripheral.hpp"

class FamilyBasicKeyboard : public Peripheral {
public:
  void reset() {
    row = 0;
  }

  const std::array<SDL_Scancode, 72> scancodes = {
      SDL_SCANCODE_F8, SDL_SCANCODE_RETURN, SDL_SCANCODE_LEFTBRACKET, SDL_SCANCODE_RIGHTBRACKET,
      SDL_SCANCODE_F10, SDL_SCANCODE_F11, SDL_SCANCODE_BACKSLASH, SDL_SCANCODE_STOP,
      SDL_SCANCODE_F7, SDL_SCANCODE_KP_AT, SDL_SCANCODE_KP_COLON, SDL_SCANCODE_SEMICOLON,
      SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_SLASH, SDL_SCANCODE_MINUS, SDL_SCANCODE_UNKNOWN,
      SDL_SCANCODE_F6, SDL_SCANCODE_O, SDL_SCANCODE_L, SDL_SCANCODE_K, SDL_SCANCODE_PERIOD,
      SDL_SCANCODE_COMMA, SDL_SCANCODE_P, SDL_SCANCODE_0,
      SDL_SCANCODE_F5, SDL_SCANCODE_I, SDL_SCANCODE_U, SDL_SCANCODE_J, SDL_SCANCODE_M,
      SDL_SCANCODE_N, SDL_SCANCODE_9, SDL_SCANCODE_8,
      SDL_SCANCODE_F4, SDL_SCANCODE_Y, SDL_SCANCODE_G, SDL_SCANCODE_H, SDL_SCANCODE_B,
      SDL_SCANCODE_V, SDL_SCANCODE_7, SDL_SCANCODE_6,
      SDL_SCANCODE_F3, SDL_SCANCODE_T, SDL_SCANCODE_R, SDL_SCANCODE_D, SDL_SCANCODE_F,
      SDL_SCANCODE_C, SDL_SCANCODE_5, SDL_SCANCODE_4,
      SDL_SCANCODE_F2, SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A,
      SDL_SCANCODE_X, SDL_SCANCODE_Z, SDL_SCANCODE_E, SDL_SCANCODE_3,
      SDL_SCANCODE_F1, SDL_SCANCODE_ESCAPE, SDL_SCANCODE_Q, SDL_SCANCODE_LCTRL,
      SDL_SCANCODE_LSHIFT, SDL_SCANCODE_RGUI, SDL_SCANCODE_1, SDL_SCANCODE_2,
      SDL_SCANCODE_HOME, SDL_SCANCODE_UP, SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_DOWN,
      SDL_SCANCODE_SPACE, SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_INSERT
  };

  bool pressed(const SDL_Scancode key) {
    SDL_PumpEvents();

    int nkeys;
    const uint8_t* keystates = SDL_GetKeyboardState(&nkeys);
    return keystates[key];
  }

  byte keys() {
    if (row == 9) {
      return 0;
    }

    byte result = 0;
    int base = row * 8 + (col ? 4 : 0);
    for (int i = 0; i < 4; ++i) {
      if (pressed(scancodes[base + i])) {
        result |= (1 << i);
      }
    }

    return result;
  }

  byte read(word addr) {
    if (addr == 0x4017) {
      if (enabled) {
        auto result = (~keys() << 1) & 0x1e;
        return result;
      }
    }
    return 0;
  }

  void increment() {
    row = (row + 1) % 10;
  }

  void write(word addr, byte value) {
    if (addr == 0x4016) {
      auto old_col = col;
      col = (value & 2) >> 1;
      if (old_col && !col) {
        increment();
      }

      if (value & 1) {
        reset();
      }
      enabled = (value & 4);
    }
  }

private:
  byte col;
  byte row;
  bool enabled;
};

