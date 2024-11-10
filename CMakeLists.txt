set(CMAKE_TOOLCHAIN_FILE "/Users/dt0/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
cmake_minimum_required(VERSION 3.16)
project(asobitomo)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src/nes_apu)
link_directories(src/nes_apu)
link_directories(${SDL2TTF_LIBRARY_DIRS})
link_directories(/Users/dt0/vcpkg/installed/arm64-osx/lib/)

set(CMAKE_PREFIX_PATH /Users/dt0/vcpkg/packages/benchmark_arm64-osx/share/)
set(CMAKE_PREFIX_PATH /Users/dt0/vcpkg/packages/sdl2-ttf_arm64-osx/share/SDL2_ttf)

add_subdirectory(ntsc)
link_directories(ntsc)

include_directories(src ntsc cereal audio_test asobitomo)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(${SDL2TTF_INCLUDE_DIRS})
add_executable(inver src/main.cpp asobitomo/util.cc
        src/bus.cpp src/cpu6502.cpp
        src/op_names.cpp src/ppu.cpp src/output_sdl.cpp
        src/td.cpp src/tm.cpp
        src/ppu_utils.cpp
        src/ops.cpp src/mapper.hpp
        src/mapper/nes003.hpp src/mapper/nes003.cpp
        src/mapper/nes004.cpp src/mapper/nes004.hpp src/header.hpp src/header.cpp
        src/mapper/nes001.cpp src/mapper/nes001.hpp
        src/Simple_Apu.cpp src/Simple_Apu.h
        src/Sound_Queue.cpp src/Sound_Queue.h
        src/mapper/nes003.cpp src/ppu_act.hpp src/ppu_act.cpp src/braille_pix.cpp src/braille_pix.hpp src/mapper/nes002.cpp src/mapper/nes002.hpp src/shifter.hpp src/family_basic_keyboard.hpp src/dev_null.cpp src/dev_null.hpp src/cloop.hpp src/cloop.cpp
        src/renderer.hpp src/renderer_ntsc.hpp src/renderer_ntsc.cpp
        src/renderer_palette.hpp src/renderer_palette.cpp src/mapper/nes007.hpp src/output.hpp src/output_null.hpp 
        # snes/cpu5a22.cpp snes/cpu5a22.hpp snes/ops_5a22.cpp snes/ops_5a22.hpp snes/op_macros_5a22.hpp
        # snes/op_names_5a22.hpp snes/op_names_5a22.cpp snes/bus_snes.hpp snes/bus_snes.cpp snes/dma.cpp snes/dma.hpp 
        )

file(COPY res/mplus-2c-medium.ttf DESTINATION "${CMAKE_BINARY_DIR}")

target_link_libraries(inver PRIVATE gflags nes_apu ntsc)
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(inver
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
target_link_libraries(inver PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
target_link_libraries(inver PRIVATE gflags::gflags)

find_package(benchmark CONFIG REQUIRED)
target_link_libraries(inver PRIVATE benchmark::benchmark)

target_compile_options(inver PRIVATE -march=native)
target_compile_options(inver PRIVATE -Wshadow -Wnon-virtual-dtor -Wcast-align -Wreorder)

add_executable(micro bench/micro.cpp src/ppu_utils.cpp)
target_link_libraries(micro benchmark::benchmark)

# add_executable(cpu5a22_test cpu5a22_test/catch.hpp snes/cpu5a22.hpp snes/cpu5a22.cpp
#         snes/ops_5a22.hpp snes/ops_5a22.cpp snes/op_macros_5a22.hpp
#         cpu5a22_test/catch_main.cpp snes/bus_snes.cpp snes/bus_snes.hpp cpu5a22_test/tests.cpp
#         cpu5a22_test/utils.hpp
#         snes/op_names_5a22.hpp snes/op_names_5a22.cpp snes/dma.cpp snes/dma.hpp snes/logger.hpp
#         snes/td.hpp snes/td.cpp snes/screen.cpp snes/screen.hpp snes/sppu.cpp snes/sppu.hpp rhomb/fort.hpp rhomb/fort.c snes/ppu_utils.cpp snes/ppu_utils.hpp)
# target_link_libraries(cpu5a22_test
#         PRIVATE
#         $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
#         $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
# )
# target_link_libraries(cpu5a22_test PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
# target_link_libraries(cpu5a22_test PRIVATE gflags nes_apu ntsc)

include_directories(${ZIP_INCLUDE_DIRS} rhomb)
link_directories(${ZIP_LIBRARY_DIRS})
add_executable(rhomb rhomb/main.cpp src/header.cpp src/header.hpp rhomb/fort.c rhomb/fort.hpp)
target_link_libraries(rhomb PRIVATE gflags::gflags zip bz2 deflate -lz)
find_package(libdeflate CONFIG REQUIRED)
target_link_libraries(rhomb PRIVATE $<IF:$<TARGET_EXISTS:libdeflate::libdeflate_shared>,libdeflate::libdeflate_shared,libdeflate::libdeflate_static>)

add_executable(snes snes/main.cpp snes/bus_snes.hpp snes/bus_snes.cpp
        snes/cpu5a22.hpp snes/cpu5a22.cpp snes/ops_5a22.hpp
        snes/ops_5a22.cpp snes/op_macros_5a22.hpp
        snes/op_names_5a22.hpp snes/op_names_5a22.cpp
        snes/snes_spc/dsp.h snes/snes_spc/dsp.cpp
        snes/snes_spc/blargg_source.h snes/snes_spc/blargg_common.h
        snes/snes_spc/blargg_config.h snes/snes_spc/blargg_endian.h
        snes/snes_spc/SNES_SPC.h snes/snes_spc/SNES_SPC.cpp
        snes/snes_spc/SNES_SPC_misc.cpp snes/snes_spc/SNES_SPC_state.cpp
        snes/snes_spc/spc.h snes/snes_spc/spc.cpp rhomb/fort.hpp rhomb/fort.c
        snes/snes_spc/SPC_CPU.h snes/snes_spc/SPC_DSP.h snes/snes_spc/SPC_DSP.cpp
        snes/snes_spc/SPC_Filter.h snes/snes_spc/SPC_Filter.cpp snes/sppu.cpp snes/sppu.hpp snes/dma.cpp snes/dma.hpp snes/logger.hpp
        snes/td.hpp snes/td.cpp snes/screen.cpp snes/screen.hpp snes/ppu_utils.hpp snes/ppu_utils.cpp
        src/peripheral.hpp snes/sdl_snes_input.hpp snes/debug.hpp snes/regs.hpp snes/debug.cpp)


target_link_libraries(snes PRIVATE gflags nes_apu ntsc)
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/Users/dt0/vcpkg/installed/arm64-osx/share/sdl2")
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(snes
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)
target_link_libraries(snes PRIVATE $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>)
target_link_libraries(snes PRIVATE gflags::gflags)