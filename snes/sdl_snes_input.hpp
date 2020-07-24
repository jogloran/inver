#pragma once

#include <SDL2/SDL.h>
#include <iostream>

#include "types.h"
#include "peripheral.hpp"

enum class SNESButtons : word {
  RT = 1 << 4,
  LT = 1 << 5,
  X = 1 << 6,
  A = 1 << 7,
  R = 1 << 8,
  L = 1 << 9,
  D = 1 << 10,
  U = 1 << 11,
  Start = 1 << 12,
  Select = 1 << 13,
  Y = 1 << 14,
  B = 1 << 15,
  None = 0,
};


inline SNESButtons operator|(SNESButtons a, SNESButtons b);

inline SNESButtons& operator|=(SNESButtons& me, SNESButtons other);

class SDLSNESController : public Peripheral {
public:
  SDLSNESController(): controller_polling(false) {
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    if (SDL_NumJoysticks() >= 1) {
      std::cout << "Pad detected" << std::endl;
      pad = SDL_GameControllerOpen(0);
    } else {
      std::cout << "Keyboard" << std::endl;
    }
  }

  ~SDLSNESController() {
    if (pad != nullptr) SDL_GameControllerClose(pad);
  }
  
  byte read(word addr) {
    byte lsb = controller_state & 1;
    controller_state >>= 1;
    return lsb;
  }
  
  void write(word addr, byte value) {
    bool controller_polling_req = (value & 0x7) == 0x1;
    if (controller_polling && !controller_polling_req) {
      controller_state = sample_input();
    } else if (!controller_polling && controller_polling_req) {
      controller_polling = true;
    }
  }
  
  word sample_input() {
    poll();
    return static_cast<word>(state);
  }

  void push_key(SDL_Keycode k) {
    SDL_Event e {.type = SDL_KEYDOWN};
    e.key.keysym.sym = k;
    SDL_PushEvent(&e);
  }

  SNESButtons pad_state() {
    state = SNESButtons::None;
    if (!pad) return state;

    if (down(SDL_CONTROLLER_BUTTON_X)) state |= SNESButtons::X;
    if (down(SDL_CONTROLLER_BUTTON_Y)) state |= SNESButtons::Y;
    if (down(SDL_CONTROLLER_BUTTON_START)) state |= SNESButtons::Start;
    if (down(SDL_CONTROLLER_BUTTON_BACK)) state |= SNESButtons::Select;
    if (down(SDL_CONTROLLER_BUTTON_A)) state |= SNESButtons::A;
    if (down(SDL_CONTROLLER_BUTTON_B)) state |= SNESButtons::B;
    if (down(SDL_CONTROLLER_BUTTON_DPAD_DOWN)) state |= SNESButtons::D;
    if (down(SDL_CONTROLLER_BUTTON_DPAD_LEFT)) state |= SNESButtons::L;
    if (down(SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) state |= SNESButtons::R;
    if (down(SDL_CONTROLLER_BUTTON_DPAD_UP)) state |= SNESButtons::U;
    if (down(SDL_CONTROLLER_BUTTON_X)) push_key(SDLK_v);
    if (down(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) state |= SNESButtons::RT;
    if (down(SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) state |= SNESButtons::LT;

    return state;
  }

  bool down(SDL_GameControllerAxis ax) const { return SDL_GameControllerGetAxis(pad, ax) > 0; }

  bool down(SDL_GameControllerButton b) const { return SDL_GameControllerGetButton(pad, b); }

  void poll() {
    SDL_PumpEvents();

    int nkeys;
    const uint8_t* keystates = SDL_GetKeyboardState(&nkeys);

    state = SNESButtons::None;
    if (keystates[SDL_SCANCODE_MINUS]) state |= SNESButtons::LT;
    if (keystates[SDL_SCANCODE_EQUALS]) state |= SNESButtons::RT;
    if (keystates[SDL_SCANCODE_RETURN]) state |= SNESButtons::Start;
    if (keystates[SDL_SCANCODE_RSHIFT]) state |= SNESButtons::Select;
    if (keystates[SDL_SCANCODE_UP]) state |= SNESButtons::U;
    if (keystates[SDL_SCANCODE_DOWN]) state |= SNESButtons::D;
    if (keystates[SDL_SCANCODE_LEFT]) state |= SNESButtons::L;
    if (keystates[SDL_SCANCODE_RIGHT]) state |= SNESButtons::R;
    if (keystates[SDL_SCANCODE_SPACE]) state |= SNESButtons::A;
    if (keystates[SDL_SCANCODE_LSHIFT]) state |= SNESButtons::B;
    if (keystates[SDL_SCANCODE_ESCAPE]) state |= SNESButtons::X;
    if (keystates[SDL_SCANCODE_TAB]) state |= SNESButtons::Y;

    if (pad) {
      state |= pad_state();
    }
  }

  SNESButtons state;
  SDL_GameController* pad = NULL;
  
  word controller_state;
  bool controller_polling;
};

inline SNESButtons operator|(SNESButtons a, SNESButtons b) {
  return static_cast<SNESButtons>(static_cast<int>(a) | static_cast<int>(b));
}

inline SNESButtons operator&(SNESButtons a, SNESButtons b) {
  return static_cast<SNESButtons>(static_cast<int>(a) & static_cast<int>(b));
}

inline SNESButtons& operator|=(SNESButtons& me, SNESButtons other) {
  return me = me | other;
}
