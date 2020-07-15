cmake_minimum_required(VERSION 3.16)
project(asobitomo)

include(FindPkgConfig)

set(gflags_DIR /usr/local/Cellar/gflags/2.2.2/lib/cmake/gflags)
find_package(gflags REQUIRED)
find_package(SDL2 REQUIRED)
find_package(benchmark REQUIRED)
pkg_search_module(SDL2TTF REQUIRED SDL2_ttf>=2.0.0)
#pkg_search_module(zip REQUIRED zip>=1.7.1)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src/nes_apu)
link_directories(src/nes_apu)
link_directories(${SDL2TTF_LIBRARY_DIRS})

add_subdirectory(ntsc)
link_directories(ntsc)

include_directories(src ntsc cereal audio_test asobitomo ${gflags_INCLUDE_DIR} /usr/local/Cellar/gflags/2.2.2/include /usr/local/Cellar/sdl2/2.0.12_1/include /usr/local/Cellar/libzip/1.7.1/include)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2TTF_INCLUDE_DIRS})
add_executable(inver src/main.cpp asobitomo/util.cc
        src/bus.cpp src/cpu6502.cpp
        src/op_names.cpp src/ppu.cpp src/output_sdl.cpp
        src/td.cpp src/tm.cpp
        src/utils.cpp
        src/ops.cpp src/mapper.hpp
        src/mapper/nes003.hpp src/mapper/nes003.cpp
        src/mapper/nes004.cpp src/mapper/nes004.hpp src/header.hpp src/header.cpp
        src/mapper/nes001.cpp src/mapper/nes001.hpp
        src/Simple_Apu.cpp src/Simple_Apu.h
        src/Sound_Queue.cpp src/Sound_Queue.h
        src/mapper/nes003.cpp src/ppu_act.hpp src/ppu_act.cpp src/braille_pix.cpp src/braille_pix.hpp src/mapper/nes002.cpp src/mapper/nes002.hpp src/shifter.hpp src/family_basic_keyboard.hpp src/dev_null.cpp src/dev_null.hpp src/cloop.hpp src/cloop.cpp
        src/renderer.hpp src/renderer_ntsc.hpp src/renderer_ntsc.cpp
        src/renderer_palette.hpp src/renderer_palette.cpp src/mapper/nes007.hpp src/output.hpp src/output_null.hpp snes/cpu5a22.cpp snes/cpu5a22.hpp snes/ops_5a22.cpp snes/ops_5a22.hpp snes/op_macros_5a22.hpp)

file(COPY res/mplus-2c-medium.ttf DESTINATION "${CMAKE_BINARY_DIR}")

target_link_libraries(inver gflags)
target_link_libraries(inver nes_apu)
target_link_libraries(inver ntsc)
target_link_libraries(inver ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES})

target_compile_options(inver PRIVATE -march=native)
target_compile_options(inver PRIVATE -Wshadow -Wnon-virtual-dtor -Wcast-align -Wreorder)

add_executable(micro bench/micro.cpp src/utils.cpp)
target_link_libraries(micro benchmark::benchmark)

add_executable(cpu5a22_test cpu5a22_test/catch.hpp snes/cpu5a22.hpp snes/cpu5a22.cpp
        snes/ops_5a22.hpp snes/ops_5a22.cpp snes/op_macros_5a22.hpp
        cpu5a22_test/catch_main.cpp snes/bus_snes.cpp snes/bus_snes.hpp cpu5a22_test/tests.cpp
        cpu5a22_test/utils.hpp
        snes/op_names_5a22.hpp snes/op_names_5a22.cpp)

include_directories(${ZIP_INCLUDE_DIRS})
link_directories(${ZIP_LIBRARY_DIRS})
add_executable(rhomb rhomb/main.cpp src/header.cpp src/header.hpp rhomb/fort.c rhomb/fort.hpp)
target_link_libraries(rhomb gflags -lzip)

add_executable(snes snes/main.cpp snes/bus_snes.hpp snes/bus_snes.cpp
        snes/cpu5a22.hpp snes/cpu5a22.cpp snes/ops_5a22.hpp
        snes/ops_5a22.cpp snes/op_macros_5a22.hpp
        snes/op_names_5a22.hpp snes/op_names_5a22.cpp
        snes/snes_spc/dsp.h snes/snes_spc/dsp.cpp
        snes/snes_spc/blargg_source.h snes/snes_spc/blargg_common.h
        snes/snes_spc/blargg_config.h snes/snes_spc/blargg_endian.h
        snes/snes_spc/SNES_SPC.h snes/snes_spc/SNES_SPC.cpp
        snes/snes_spc/SNES_SPC_misc.cpp snes/snes_spc/SNES_SPC_state.cpp
        snes/snes_spc/spc.h snes/snes_spc/spc.cpp
        snes/snes_spc/SPC_CPU.h snes/snes_spc/SPC_DSP.h snes/snes_spc/SPC_DSP.cpp
        snes/snes_spc/SPC_Filter.h snes/snes_spc/SPC_Filter.cpp)

target_link_libraries(snes gflags)
target_link_libraries(snes ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES} ${SDL2TTF_LIBRARIES})