#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <iterator>

#include "../snes/cpu5a22.hpp"
#include "../snes/bus_snes.hpp"
#include "utils.hpp"

using namespace std::literals;

std::vector<char> assemble(std::string text) {
  char out_fn_bytes[] = "/tmp/cpu5a22_outXXXXXX";
  ::mkstemp(out_fn_bytes);
  auto out_fn = std::string(out_fn_bytes);

  char inp_fn_bytes[] = "/tmp/cpu5a22_inpXXXXXX";
  ::mkstemp(inp_fn_bytes);
  auto inp_fn = std::string(inp_fn_bytes);

  if (std::ofstream prg(inp_fn); prg) {
    prg << "* = $0000\n"
        << text;
  }

  auto cmd = "acme -o "s + out_fn + " --cpu 65816 " + inp_fn;
  std::system(cmd.c_str());

  std::vector<char> result;
  if (std::ifstream f(out_fn); f) {
    std::copy(std::istreambuf_iterator<char>(f),
              std::istreambuf_iterator<char>(),
              std::back_inserter(result));
  }

  return result;
}

void dump(std::vector<char> bytes) {
  for (byte b : bytes) {
    std::printf("%02x ", byte(b));
  }
  std::printf("\n");
}

std::shared_ptr<BusSNES> run(std::string text) {
  auto bytes = assemble(text);
  dump(bytes);
  auto bus = std::make_unique<BusSNES>();
  std::copy(bytes.begin(), bytes.end(), bus->ram.begin());
  auto eop = bytes.size();

  while (true) {
    bus->cpu->tick();
    if (bus->cpu->pc.addr >= eop) {
      break;
    }
  }

  return bus;
}
