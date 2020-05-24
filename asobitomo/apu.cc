#include "apu.h"
#include <SDL2/SDL.h>
#include <iomanip>

bool
APU::set(word loc, byte value) {  
  switch (loc) {
    case 0xff10:
      ch1.set_sweep(value);
      break;
    case 0xff11:
      ch1.set_length_and_duty(value);
      break;
    case 0xff12:
      ch1.set_envelope(value);
      break;
    case 0xff13:
      ch1.set_frequency_low(value);
      break;
    case 0xff14:
      ch1.set_frequency_high(value);
      break;
    case 0xff16:
      ch2.set_length_and_duty(value);
      break;
    case 0xff17:
      ch2.set_envelope(value);
      break;
    case 0xff18:
      ch2.set_frequency_low(value);
      break;
    case 0xff19:
      ch2.set_frequency_high(value);
      break;
    case 0xff1a:
      ch3.set_enabled(value);
      break;
    case 0xff1b:
      ch3.set_length(value);
      break;
    case 0xff1c:
      ch3.set_output_level(value);
      break;
    case 0xff1d:
      ch3.set_wave_low(value);
      break;
    case 0xff1e:
      ch3.set_wave_high_and_control(value);
      break;
    case 0xff30:
    case 0xff31:
    case 0xff32:
    case 0xff33:
    case 0xff34:
    case 0xff35:
    case 0xff36:
    case 0xff37:
    case 0xff38:
    case 0xff39:
    case 0xff3a:
    case 0xff3b:
    case 0xff3c:
    case 0xff3d:
    case 0xff3e:
    case 0xff3f:
      ch3.set_wave_pattern(value, loc - 0xff30);
      break;
    case 0xff20:
      ch4.set_length(value);
      break;
    case 0xff21:
      ch4.set_volume_envelope(value);
      break;
    case 0xff22:
      ch4.set_polynomial_counter(value);
      break;
    case 0xff23:
      ch4.set_counter_consecutive(value);
      break;
    case 0xff24:
      set_channel_control(value);
      break;
    case 0xff25:
      set_sound_output(value);
      break;
    case 0xff26:
      set_channel_levels(value);
      break;
      
    default:
      return false;
  }
  
  return true;
}

byte*
APU::get(word loc) {
  switch (loc) {
    case 0xff10:
    case 0xff11:
    case 0xff12:
    case 0xff13:
    case 0xff14:
      return &ch1.values[loc - 0xff10];
      break;
      
    case 0xff16:
    case 0xff17:
    case 0xff18:
    case 0xff19:
      return &ch2.values[loc - 0xff16];
      break;
      
    case 0xff1a:
    case 0xff1b:
    case 0xff1c:
    case 0xff1d:
    case 0xff1e:
      return &ch3.values[loc - 0xff1a];
      break;
      
    case 0xff20:
    case 0xff21:
    case 0xff22:
    case 0xff23:
      return &ch4.values[loc - 0xff20];
      break;
  }
  
  return nullptr;
}

void
APU::step(long delta) {
  for (int n = 0; n < delta; ++n) {
    //  std::cout << seq << ' ' << sample_timer << std::endl;
    ch1.tick();
    ch2.tick();
    ch3.tick();
    ch4.tick();
    
    if (seq && seq-- == 0) {
      if (seq_step % 2 == 0) {
        ch1.tick_length();
        ch2.tick_length();
        ch3.tick_length();
        ch4.tick_length();
      }
      
      if (seq_step == 7) {
        ch1.tick_volume();
        ch2.tick_volume();
        ch4.tick_volume();
      }
      
      if (seq_step == 2 || seq_step == 6) {
        ch1.tick_sweep();
      }
      
      seq_step = (seq_step + 1) % 8;
      seq = 8192;
    }
    
    if (sample_timer-- == 0) {
      //    std::cout << "sample" << std::endl;
      int16_t L = 0, R = 0;
      int16_t v1 = ch1();
      int16_t v2 = ch2();
      int16_t v3 = ch3();
      int16_t v4 = ch4();
      L += v1;
      R += v1;
      L += v2;
      R += v2;
      L += v3;
      R += v3;
      L += v4;
      R += v4;
      
      L *= (left.volume << 3);
      R *= (right.volume << 3);
      
      *buf_ptr++ = L;
      *buf_ptr++ = R;
      
      if (buf_ptr >= buf.end()) {
        buf_ptr = buf.begin();
        
        while (SDL_GetQueuedAudioSize(dev) > buf.size() * sizeof(int16_t)) {
          SDL_Delay(1);
        }
        
        SDL_QueueAudio(dev, (void*)buf.data(), static_cast<uint32_t>(buf.size()) * sizeof(int16_t));
      }
      
      sample_timer = CYCLES_PER_SECOND / SAMPLE_RATE;
    }
  }
}

void
APU::sdl_setup() {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  
  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = SAMPLE_RATE;
  want.format = AUDIO_S16;
  want.channels = NCHANNELS;
  want.samples = BUFSIZE;
  want.callback = NULL;
  
  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
  SDL_PauseAudioDevice(dev, 0);
}
