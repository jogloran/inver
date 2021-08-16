#include <memory>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <set>

#include <gflags/gflags.h>
#include <map>

#include "rang.hpp"

#include "bus_snes.hpp"
#include "debug.hpp"
#include "mapper/hirom.hpp"
#include "mapper/lorom.hpp"
#include "metadata.hpp"
#include "types.h"

#include "flags.h"

extern std::set<std::string> active_tags;
std::map<dword, PCWatchSpec> dis_pcs;
std::set<dword> ignored_pcs;
std::vector<ChangeWatchSpec> change_watches;

std::vector<byte> read_bytes(std::ifstream& f) {
  return {std::istreambuf_iterator<char>(f),
          std::istreambuf_iterator<char>()};
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
  Metadata meta = interpret(data);
  inspect(meta);

  std::shared_ptr<Mapper> mapper;
  switch (meta.mapper) {
    case Metadata::Mapper::HiROM:
      mapper = std::make_shared<HiROM>();
      break;
    case Metadata::Mapper::LoROM:
      mapper = std::make_shared<LoROM>();
      break;
    default:
      throw std::runtime_error("Unsupported mapper");
  }

  mapper->map(std::move(data));
  bus.connect(mapper);

  if (FLAGS_load_save) {
    bus.unpickle("save.state");
  } else {
    bus.reset();
    bus.set_pc(meta.rst);
  }
  bus.run();
}
