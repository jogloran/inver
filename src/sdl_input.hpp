#pragma once

#include <SDL2/SDL.h>

#include "types.h"

enum class Buttons : byte {
  R = 1 << 7,
  L = 1 << 6,
  D = 1 << 5,
  U = 1 << 4,
  Start = 1 << 3,
  Select = 1 << 2,
  B = 1 << 1,
  A = 1 << 0,
  None = 0,
};


inline Buttons operator|(Buttons a, Buttons b);

inline Buttons& operator|=(Buttons& me, Buttons other);

class SDLInput {
public:
  SDLInput() {
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    if (SDL_NumJoysticks() >= 1) {
      std::cout << "Pad detected" << std::endl;
      pad = SDL_GameControllerOpen(0);
    }
  }

  ~SDLInput() {
    if (pad != nullptr) SDL_GameControllerClose(pad);
  }

  void push_key(SDL_Keycode k) {
    SDL_Event e {.type = SDL_KEYDOWN};
    e.key.keysym.sym = k;
    SDL_PushEvent(&e);
  }

  Buttons pad_state() {
    state = Buttons::None;
    if (!pad) return state;

    if (down(SDL_CONTROLLER_BUTTON_START)) state |= Buttons::Start;
    if (down(SDL_CONTROLLER_BUTTON_BACK)) state |= Buttons::Select;
    if (down(SDL_CONTROLLER_BUTTON_A)) state |= Buttons::A;
    if (down(SDL_CONTROLLER_BUTTON_B)) state |= Buttons::B;
    if (down(SDL_CONTROLLER_BUTTON_DPAD_DOWN)) state |= Buttons::D;
    if (down(SDL_CONTROLLER_BUTTON_DPAD_LEFT)) state |= Buttons::L;
    if (down(SDL_CONTROLLER_BUTTON_DPAD_RIGHT)) state |= Buttons::R;
    if (down(SDL_CONTROLLER_BUTTON_DPAD_UP)) state |= Buttons::U;
    if (down(SDL_CONTROLLER_BUTTON_X)) push_key(SDLK_v);
    if (down(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) push_key(SDLK_e);
    if (down(SDL_CONTROLLER_AXIS_TRIGGERLEFT)) push_key(SDLK_e);

    return state;
  }

  bool down(SDL_GameControllerAxis ax) const { return SDL_GameControllerGetAxis(pad, ax) > 0; }

  bool down(SDL_GameControllerButton b) const { return SDL_GameControllerGetButton(pad, b); }

  void poll() {
    SDL_PumpEvents();

    int nkeys;
    const uint8_t* keystates = SDL_GetKeyboardState(&nkeys);

    state = Buttons::None;
    if (keystates[SDL_SCANCODE_RETURN]) state |= Buttons::Start;
    if (keystates[SDL_SCANCODE_RSHIFT]) state |= Buttons::Select;
    if (keystates[SDL_SCANCODE_UP]) state |= Buttons::U;
    if (keystates[SDL_SCANCODE_DOWN]) state |= Buttons::D;
    if (keystates[SDL_SCANCODE_LEFT]) state |= Buttons::L;
    if (keystates[SDL_SCANCODE_RIGHT]) state |= Buttons::R;
    if (keystates[SDL_SCANCODE_SPACE]) state |= Buttons::A;
    if (keystates[SDL_SCANCODE_LSHIFT]) state |= Buttons::B;

    if (pad) {
      state |= pad_state();
    }
  }

  Buttons state;
  SDL_GameController* pad = NULL;
};

inline Buttons operator|(Buttons a, Buttons b) {
  return static_cast<Buttons>(static_cast<int>(a) | static_cast<int>(b));
}

inline Buttons operator&(Buttons a, Buttons b) {
  return static_cast<Buttons>(static_cast<int>(a) & static_cast<int>(b));
}

inline Buttons& operator|=(Buttons& me, Buttons other) {
  return me = me | other;
}
