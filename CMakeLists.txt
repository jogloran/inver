cmake_minimum_required(VERSION 3.16)
project(asobitomo)
set(gflags_DIR /usr/local/Cellar/gflags/2.2.2/lib/cmake/gflags)
find_package(gflags REQUIRED)
find_package(SDL2 REQUIRED)

set(CMAKE_CXX_STANDARD 17)

include_directories(inver audio_test asobitomo ${gflags_INCLUDE_DIR} /usr/local/Cellar/gflags/2.2.2/include /usr/local/Cellar/sdl2/2.0.12/include)
include_directories(${SDL2_INCLUDE_DIRS})
add_executable(inver inver/main.cpp asobitomo/util.cc inver/bus.cpp inver/mapper/nes000.cpp inver/cpu6502.cpp inver/op_names.cpp inver/ppu.cpp inver/screen.cpp inver/sdl_input.cpp inver/tm.cpp inver/utils.cpp inver/ops.cpp inver/mapper.hpp inver/mapper/nes004.cpp inver/mapper/nes004.hpp inver/header.hpp inver/header.cpp inver/mapper/nes001.cpp inver/mapper/nes001.hpp)
target_link_libraries(inver gflags)
target_link_libraries(inver ${SDL2_LIBRARIES})