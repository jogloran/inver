#pragma once

#include <SDL2/SDL.h>

enum class Buttons : int {
  U = 1 << 7,
  D = 1 << 6,
  L = 1 << 5,
  R = 1 << 4,
  Select = 1 << 3,
  Start = 1 << 2,
  B = 1 << 1,
  A = 1 << 0,
  None = 0,
};
  

inline Buttons operator|(Buttons a, Buttons b);
inline Buttons& operator|=(Buttons& me, Buttons other);

class SDLInput {
public:
  void poll() {
    SDL_PumpEvents();
    
    int nkeys;
    const uint8_t* keystates = SDL_GetKeyboardState(&nkeys);
    
    state = Buttons::None;
    if (keystates[SDL_SCANCODE_RETURN]) {
      state |= Buttons::Start;
    }
    if (keystates[SDL_SCANCODE_RSHIFT]) {
      state |= Buttons::Select;
    }
    if (keystates[SDL_SCANCODE_UP]) {
      state |= Buttons::U;
    }
    if (keystates[SDL_SCANCODE_DOWN]) {
      state |= Buttons::D;
    }
    if (keystates[SDL_SCANCODE_LEFT]) {
      state |= Buttons::L;
    }
    if (keystates[SDL_SCANCODE_RIGHT]) {
      state |= Buttons::R;
    }
    if (keystates[SDL_SCANCODE_SPACE]) {
      state |= Buttons::A;
    }
    if (keystates[SDL_SCANCODE_LSHIFT]) {
      state |= Buttons::B;
    }
  }
  
  Buttons state;
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
