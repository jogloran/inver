#include <benchmark/benchmark.h>
#include "types.h"
#include "utils.hpp"

#include <array>

std::array<byte, 8> naive_unpack_bits(byte lsb, byte msb) {
  std::array<byte, 8> result;
  for (int i = 0; i < 8; ++i) {
    result[7 - i] = (lsb & 1) | ((msb & 1) << 1);
    lsb >>= 1;
    msb >>= 1;
  }
  return result;
}

static void Unpack(benchmark::State& s) {
  for (auto _ : s) {
    byte lsb = rand() % 0xff;
    byte msb = rand() % 0xff;
    unpack_bits(lsb, msb);
  }
}

static void UnpackNaive(benchmark::State& s) {
  for (auto _ : s) {
    byte lsb = rand() % 0xff;
    byte msb = rand() % 0xff;
    naive_unpack_bits(lsb, msb);
  }
}

static void CompareUnpack(benchmark::State& s) {
  for (auto _ : s) {
    byte lsb = rand() % 0xff;
    byte msb = rand() % 0xff;
    auto a = unpack_bits(lsb, msb);
    auto b = naive_unpack_bits(lsb, msb);
    for (int i = 0; i < 8; ++i) {
      assert(a[i] == b[i]);
    }
  }
}

BENCHMARK(Unpack);
BENCHMARK(UnpackNaive);
BENCHMARK(CompareUnpack);

BENCHMARK_MAIN();