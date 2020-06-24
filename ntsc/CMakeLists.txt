cmake_minimum_required(VERSION 3.16)
project(ntsc)
set(CMAKE_CXX_STANDARD 17)
find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})
include_directories(/usr/local/Cellar/sdl2/2.0.12_1/include)

add_library(ntsc SHARED nes_ntsc_config.h nes_ntsc.h nes_ntsc.c nes_ntsc_impl.h)

add_executable(demo demo.c demo_impl.h)
target_link_libraries(demo ntsc ${SDL2_LIBRARIES})