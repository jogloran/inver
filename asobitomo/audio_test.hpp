#pragma once
#include <SDL2/SDL.h>
#include <algorithm>
#include <array>
#include <iostream>

#include <stdint.h>

int main(int argc, char* argv[])
{
  //  if (SDL_Init(SDL_INIT_AUDIO) != 0) {
  //    SDL_Log("Unable to initialize SDL: %s\n", SDL_GetError());
  //    return 1;
  //  }
  //
  //  SDL_AudioSpec want, have;
  //  SDL_AudioDeviceID dev;
  //
  //  SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
  //  want.freq = 48000;
  //  want.format = AUDIO_S16;
  //  want.channels = 2;
  //  want.samples = 1024;
  //  want.callback = NULL;
  //
  //  int32_t len = want.samples * want.channels;
  //  int16_t data[len];
  //  int16_t tone_volume = 3000;
  //  int32_t period = 128;
  //  int32_t i;
  //  for (i=0; i < len; ++i) {
  //    if ((i/ period) % 2 == 0) {
  //      data[i] = tone_volume * (float(i)/period);
  //    } else {
  //      data[i] = -tone_volume * (float(i)/period);
  //    }
  //    std::cout<<data[i] << ' ';
  //  }
  //  std::cout<<std::endl;
  //
  //  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
  //  if (dev == 0) {
  //    SDL_Log("Failed to open audio: %s", SDL_GetError());
  //  } else {
  //    if (have.format != want.format) { /* we let this one thing change. */
  //      SDL_Log("We didn't get Float32 audio format.");
  //    }
  //
  //    SDL_Log("%d %d %d %d", want.samples, have.samples, want.size, have.size);
  //
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //    SDL_QueueAudio(dev, data, sizeof(data));
  //
  //    SDL_PauseAudioDevice(dev, 0); /* start audio playing. */
  //    SDL_Delay(1000); /* let the audio callback play some sound for 5 seconds. */
  //
  //
  //
  //
  //    SDL_CloseAudioDevice(dev);
  //  }
  //
  //  SDL_Quit();
  //
  //  return 0;
  //}
  
  //int main() {
  if (SDL_Init(SDL_INIT_AUDIO) != 0) {
    printf("SDL failed to open\n");
    return 1;
  }
  
  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;
  
  SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
  want.freq = 48000;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = 1024;
  want.callback = NULL;
  
  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
  if (dev == 0) {
    SDL_Log("Failed to open audio: %s", SDL_GetError());
  } else {
    if (have.format != want.format) { /* we let this one thing change. */
      SDL_Log("We didn't get Float32 audio format.");
    }
  }
  
  SDL_Log("%d %d %d %d", want.samples, have.samples, want.size, have.size);
  
  int32_t len = 48000;
  int16_t data[len];
  int16_t vol = 1000;
  int32_t period = 64;
  int32_t i;
  
  int duty = 0xc0;
  for (i=0; i < len; ++i) {
    data[i] = (duty & 0x1) ? vol : 0;
    data[i] = (duty & 0x1) ? vol : 0;
    
    if (i % 24 == 0) {
    duty >>= 1;
    if (duty == 0) duty = 0xc0;
    }
  }
  
  //  int samples_per_second = 48000;
  //  constexpr int bytes_per_sample = sizeof(int16_t) * 2;
  //
  //  constexpr int bytes_to_write = 48000 * bytes_per_sample; // 1/60 sec of audio
  //  std::array<int16_t, bytes_to_write> buf;
  //  auto ptr = buf.begin();
  //  bool s = false;
  //  while (ptr != buf.end()) {
  //    *ptr++ = (s?1:-1) * 5000;
  //    *ptr++ = (s?1:-1) * 5000;
  //
  //    s = !s;
  //  }
  //
  //  int sample_count = bytes_to_write / bytes_per_sample;
  
  
  SDL_QueueAudio(dev, (void*)data, sizeof(data));
  SDL_QueueAudio(dev, (void*)data, sizeof(data));
  SDL_QueueAudio(dev, (void*)data, sizeof(data));
  SDL_QueueAudio(dev, (void*)data, sizeof(data));
//  SDL_QueueAudio(dev, (void*)data, sizeof(data));
//  SDL_QueueAudio(dev, (void*)data, sizeof(data));
//  SDL_QueueAudio(dev, (void*)data, sizeof(data));
  SDL_PauseAudioDevice(dev, 0); /* start audio playing. */
  while (SDL_GetQueuedAudioSize(dev) > sizeof(data)) {
    SDL_Delay(1);
  }
//  }

//  SDL_Delay(5000); /* let the audio callback play some sound for 5 seconds. */
  
  //
  //  for (int sample_idx = 0; sample_idx < sample_count; ++i) {
  //
  //  }
  //
  //
  //  SDL_AudioSpec want;
  //  SDL_memset(&want, 0, sizeof(want));
  //
  //  SDL_AudioSpec have;
  //  want.freq = 48000;
  //  want.format = AUDIO_S16;
  //  want.channels = 2;
  //  want.samples = 1024;
  //
  //  SDL_AudioDeviceID sdlAudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
  //  std::cout << sdlAudioDevice << std::endl;
  //
  //  //  while (true) {
  //  SDL_QueueAudio(sdlAudioDevice, (void*)buf.data(), 1024 * 4);
  //  //  }
  //
  //  SDL_PauseAudioDevice(sdlAudioDevice, 0);    //unpause audio
  //  while (SDL_GetQueuedAudioSize(sdlAudioDevice) != 0) {
  //    SDL_Delay(1);
  //  }
  //  SDL_PauseAudioDevice(sdlAudioDevice, 1);    //pause audio playing
}

