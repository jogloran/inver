cmake_minimum_required(VERSION 3.16)
project(asobitomo)
set(gflags_DIR /usr/local/Cellar/gflags/2.2.2/lib/cmake/gflags)
find_package(gflags REQUIRED)
find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src/nes_apu)
link_directories(src/nes_apu)

include_directories(src audio_test asobitomo ${gflags_INCLUDE_DIR} /usr/local/Cellar/gflags/2.2.2/include /usr/local/Cellar/sdl2/2.0.12_1/include)
include_directories(${SDL2_INCLUDE_DIRS})
add_executable(inver src/main.cpp asobitomo/util.cc
        src/bus.cpp src/cpu6502.cpp
        src/op_names.cpp src/ppu.cpp src/screen.cpp
        src/td.cpp src/tm.cpp
        src/utils.cpp
        src/ops.cpp src/mapper.hpp
        src/mapper/nes003.hpp src/mapper/nes003.cpp
        src/mapper/nes004.cpp src/mapper/nes004.hpp src/header.hpp src/header.cpp
        src/mapper/nes001.cpp src/mapper/nes001.hpp
        src/Simple_Apu.cpp src/Simple_Apu.h src/Sound_Queue.cpp src/Sound_Queue.h src/mapper/nes003.cpp)

target_link_libraries(inver gflags)
target_link_libraries(inver nes_apu)
target_link_libraries(inver ${SDL2_LIBRARIES})