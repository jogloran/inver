#include <memory>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <set>

#include <gflags/gflags.h>
#include <map>

#include "rang.hpp"

#include "types.h"
#include "bus_snes.hpp"
#include "debug.hpp"

DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(xx, false, "Debug");
DEFINE_bool(dump_stack, false, "Dump stack");
DEFINE_bool(audio, false, "Enable audio");
DEFINE_bool(test_rom_output, false, "Test ROM output hack");
DEFINE_string(tags, "", "Log tags to print, separated by commas");
DEFINE_bool(td, false, "Show tile debugger (nametable)");
DEFINE_bool(show_raster, false, "Show raster");
DEFINE_string(dis_pcs, "", "ROM locations to dump for");
DEFINE_string(ignored_pcs, "", "ROM locations to not dump for");
DEFINE_string(change_watches, "", "Memory locations to watch for changes at");
DEFINE_bool(fake_sprites, false, "Displays a red rectangle at the top left of each sprite");
DEFINE_int32(sram, 0x10000, "Size of the first SRAM bank (70-7d:0000-ffff)");
DEFINE_bool(load_save, false, "Load save immediately");

std::set<std::string> active_tags;
std::map<dword, PCWatchSpec> dis_pcs;
std::set<dword> ignored_pcs;
std::vector<ChangeWatchSpec> change_watches;

std::vector<byte> read_bytes(std::ifstream& f) {
  return {std::istreambuf_iterator<char>(f),
          std::istreambuf_iterator<char>()};
}

word inspect(std::vector<byte> data) {
  word offset = 0x7000;
  byte* name = &data[offset + 0xfc0];
  if (std::any_of(name, name + 21, [](char c) { return !isprint(c); })) {
    offset = 0xf000;
    name = &data[offset + 0xfc0];
  }

  std::printf("%.*s\n", 21, name);
  std::printf("Type: ");
  byte rom_layout = data[offset + 0xfd5];
  if ((rom_layout & 0b110000) == 3) {
    std::printf("FastROM\n");
  } else {
    if (rom_layout & 1) std::printf("HiROM\n");
    else std::printf("LoROM\n");
  }
  std::printf("ROM: %02x\n", data[offset + 0xfd6]);
  std::printf("ROM size:  %d\n", 0x400 << data[offset + 0xfd7]);
  std::printf("SRAM size: %d\n", 0x400 << data[offset + 0xfd8]);
  std::printf("Creator:   %02x\n", byte(data[offset + 0xfd9]));
  std::printf("Version:   %02x\n", byte(data[offset + 0xfdb]));
  std::printf("Checksum:  %02x\n", byte(data[offset + 0xfde]));
  std::printf("~Checksum: %02x\n", byte(data[offset + 0xfdc]));
  std::printf("RST:       %04x\n", word(data[offset + 0xffc]) | word(data[offset + 0xffd] << 8));
  std::printf("NMI:       %04x\n", word(data[offset + 0xfea]) | word(data[offset + 0xfeb] << 8));

  return word(data[offset + 0xffc]) | word(data[offset + 0xffd] << 8);
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
  dis_pcs = parse_pc_watch_spec(FLAGS_dis_pcs);
  ignored_pcs = parse_ignored_pcs(FLAGS_ignored_pcs);
  change_watches = parse_change_watches(FLAGS_change_watches);

  BusSNES bus;
  auto s = std::make_shared<Screen>();
  bus.attach_screen(s);

  std::vector<byte> data = read_bytes(f);
  word rst = inspect(data);

  bus.map(std::move(data));

  if (FLAGS_load_save) {
    bus.unpickle("save.state");
  } else {
    bus.reset();
    bus.set_pc(rst);
  }
  bus.run();
}
