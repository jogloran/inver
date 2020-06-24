cmake_minimum_required(VERSION 3.16)
project(nes_apu)
set(CMAKE_CXX_STANDARD 17)

add_library(nes_apu SHARED Nes_Apu.h Nes_Apu.cpp blargg_common.h blargg_source.h
        Nes_Oscs.cpp Nes_Oscs.h Blip_Buffer.cpp Blip_Buffer.h Blip_Synth.h
        boost/config.hpp boost/cstdint.hpp boost/static_assert.hpp
        apu_snapshot.cpp apu_snapshot.h)