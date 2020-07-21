#include <memory>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <fstream>
#include <sstream>
#include <set>

#include <gflags/gflags.h>

#include "rang.hpp"

#include "types.h"
#include "cpu5a22.hpp"
#include "bus_snes.hpp"

DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(xx, false, "Debug");
DEFINE_bool(dump_stack, false, "Dump stack");
DEFINE_bool(audio, false, "Enable audio");
DEFINE_bool(test_rom_output, false, "Test ROM output hack");
DEFINE_string(tags, "", "Log tags to print, separated by commas");

std::set<std::string> active_tags;

std::set<std::string> parse_tags(std::string tags_str) {
  std::set<std::string> result;
  if (tags_str == "") return result;

  std::string tags(tags_str);
  std::replace(tags.begin(), tags.end(), ',', ' ');

  std::istringstream ss(tags);
  std::copy(std::istream_iterator<std::string>(ss),
            std::istream_iterator<std::string>(),
            std::inserter(result, result.begin()));

  return result;
}

std::vector<byte> read_bytes(std::ifstream& f) {
  return {std::istreambuf_iterator<char>(f),
          std::istreambuf_iterator<char>()};
}

word inspect(std::vector<byte> data) {
  byte* name = &data[0x7fc0];
  std::printf("%.*s\n", 21, name);
  std::printf("Type: ");
  byte rom_layout = data[0x7fd5];
  if ((rom_layout & 0b110000) == 3) {
    std::printf("FastROM\n");
  } else {
    if (rom_layout & 1) std::printf("HiROM\n");
    else std::printf("LoROM\n");
  }
  std::printf("ROM: %02x\n", data[0x7fd6]);
  std::printf("ROM size:  %d\n", 0x400 << data[0x7fd7]);
  std::printf("SRAM size: %d\n", 0x400 << data[0x7fd8]);
  std::printf("Creator:   %02x\n", byte(data[0x7fd9]));
  std::printf("Version:   %02x\n", byte(data[0x7fdb]));
  std::printf("Checksum:  %02x\n", byte(data[0x7fde]));
  std::printf("~Checksum: %02x\n", byte(data[0x7fdc]));
  std::printf("RST:       %04x\n", word(data[0x7ffc]) | word(data[0x7ffd] << 8));
  std::printf("NMI:       %04x\n", word(data[0x7fea]) | word(data[0x7feb] << 8));

  return word(data[0x7ffc]) | word(data[0x7ffd] << 8);
}

int main(int argc, char* argv[]) {
  gflags::SetUsageMessage("A SNES emulator");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  rang::setControlMode(rang::control::Force);

  if (argc != 2) {
    std::cerr << "Expecting a ROM filename." << std::endl;
    std::exit(1);
  }

  std::ifstream f(argv[1], std::ios::in);
  if (!f) {
    f.open(std::string(argv[1]) + ".sfc", std::ios::in);
    if (!f) {
      std::cerr << "Couldn't access ROM file." << std::endl;
      std::exit(2);
    }
  }

  active_tags = parse_tags(FLAGS_tags);

  BusSNES bus;
  auto s = std::make_shared<Screen>();
  bus.ppu.connect(s);

  std::vector<byte> data = read_bytes(f);
  word rst = inspect(data);

  bus.map(std::move(data));
  bus.reset();
  bus.cpu->pc.addr = rst;
  bus.run();
}
