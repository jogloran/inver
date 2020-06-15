cmake_minimum_required(VERSION 3.16)
project(asobitomo)

include(FindPkgConfig)

set(gflags_DIR /usr/local/Cellar/gflags/2.2.2/lib/cmake/gflags)
find_package(gflags REQUIRED)
find_package(SDL2 REQUIRED)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src/nes_apu)
link_directories(src/nes_apu)
link_directories(${SDL2TTF_LIBRARY_DIRS})

include_directories(src cereal audio_test asobitomo ${gflags_INCLUDE_DIR} /usr/local/Cellar/gflags/2.2.2/include /usr/local/Cellar/sdl2/2.0.12_1/include)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2TTF_INCLUDE_DIRS})
add_executable(inver src/main.cpp asobitomo/util.cc
        src/bus.cpp src/cpu6502.cpp
        src/op_names.cpp src/ppu.cpp src/screen.cpp
        src/td.cpp src/tm.cpp
        src/utils.cpp
        src/ops.cpp src/mapper.hpp
        src/mapper/nes003.hpp src/mapper/nes003.cpp
        src/mapper/nes004.cpp src/mapper/nes004.hpp src/header.hpp src/header.cpp
        src/mapper/nes001.cpp src/mapper/nes001.hpp
        src/Simple_Apu.cpp src/Simple_Apu.h src/Sound_Queue.cpp src/Sound_Queue.h src/mapper/nes003.cpp src/ppu_act.hpp src/ppu_act.cpp src/braille_pix.cpp src/braille_pix.hpp src/mapper/nes002.cpp src/mapper/nes002.hpp)

target_link_libraries(inver gflags)
target_link_libraries(inver nes_apu)
target_link_libraries(inver ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES})
