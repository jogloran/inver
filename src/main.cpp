#include <memory>
#include <iostream>
#include <cstdlib>

#include "mapper/nes000.hpp"
#include "mapper/nes001.hpp"
#include "mapper/nes002.hpp"
#include "mapper/nes003.hpp"
#include "mapper/nes004.hpp"
#include "mapper/nes007.hpp"
#include "types.h"
#include "bus.hpp"
#include "header.hpp"
#include "op_names.hpp"
#include "output_null.hpp"
#include "renderer_ntsc.hpp"
#include "renderer_palette.hpp"

#include <gflags/gflags.h>

std::shared_ptr<Mapper> mapper_for(byte no);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(xx, false, "Debug");
DEFINE_bool(cloop, false, "When dumping disassembly, detect and condense loops");
DEFINE_bool(show_raster, false, "Show raster in rendered output");
DEFINE_bool(fake_sprites, false, "Show fake sprites");
DEFINE_bool(dump_stack, false, "Dump stack");
DEFINE_bool(audio, false, "Enable audio");
DEFINE_string(save, "", "Save path");
DEFINE_bool(tm, false, "Show tile map");
DEFINE_bool(td, false, "Show tile debugger (nametable)");
DEFINE_int32(td_scanline, 0, "Tile debugger samples at this scanline");
DEFINE_int32(td_refresh_rate, 240, "Tile debugger updates per n PPU cycles");
DEFINE_string(ppu_log_spec, "", "PPU log spec");
DEFINE_bool(dump_ops, false, "Dump op table");
DEFINE_bool(kb, false, "Family Basic keyboard accessible over 0x4016");
DEFINE_int32(pc, -1, "Override initial pc");
DEFINE_bool(null_output, false, "Null output (implies no input devices)");
DEFINE_string(filter, "ntsc", "");

std::vector<char> read_bytes(std::ifstream& f) {
  return {std::istreambuf_iterator<char>(f),
          std::istreambuf_iterator<char>()};
}

NESHeader read_header(std::ifstream& f) {
  NESHeader h;
  f.read((char*) &h, 0x10);
  return h;
}

std::shared_ptr<Mapper>
read_mapper(const std::vector<char>& data,
            const std::vector<char>& save_data,
            const NESHeader* h) {
  auto mapper = mapper_for(mapper_no(h));
  mapper->map(data, prg_rom_size(h), chr_rom_size(h), h);
  if (save_data.size()) {
    mapper->map_ram(save_data, save_data.size());
  }
  return mapper;
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage("A NES emulator");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_dump_ops) {
    dump_op_table(std::cout);
    std::exit(0);
  }

  if (argc != 2) {
    std::cerr << "Expecting a ROM filename." << std::endl;
    std::exit(1);
  }

  std::ifstream f(argv[1], std::ios::in);
  if (!f) {
    f.open(std::string(argv[1]) + ".nes", std::ios::in);
    if (!f) {
      std::cerr << "Couldn't access ROM file." << std::endl;
      std::exit(2);
    }
  }

  std::vector<char> save_data;

  if (std::ifstream save(FLAGS_save, std::ios::in); save) {
    std::cerr << "Loading from " << FLAGS_save << std::endl;
    std::copy(std::istreambuf_iterator<char>(save),
              std::istreambuf_iterator<char>(),
              std::back_inserter(save_data));
  }

  NESHeader h = read_header(f);
  inspect_header(&h);
  std::vector<char> data = read_bytes(f);
  std::shared_ptr<Mapper> mapper = read_mapper(data, save_data, &h);

  Bus bus;
  if (FLAGS_td) {
    auto td = std::make_shared<TD>();
    bus.connect(td);
  }
  if (FLAGS_tm) {
    auto tm = std::make_shared<TM>();
    bus.connect(tm);
  }

  std::shared_ptr<Output> screen;
  if (FLAGS_null_output) {
    screen = std::make_shared<NullOutput>();
  } else {
    if (FLAGS_filter == "ntsc") {
      screen = std::make_shared<SDLOutput>(std::make_unique<NTSCRenderer>());
    } else {
      screen = std::make_shared<SDLOutput>(std::make_unique<PaletteRenderer>());
    }

    if (FLAGS_kb) {
      bus.connect2(std::make_shared<FamilyBasicKeyboard>());
    } else {
      bus.connect1(std::make_shared<SDLInput>());
    }
  }

  bus.attach_screen(screen);
  bus.attach_cart(mapper);

  bus.run();
}

std::shared_ptr<Mapper> mapper_for(byte no) {
  switch (no) {
    case 0:
      return std::make_shared<NROM>();
    case 1:
      return std::make_shared<MMC1>();
    case 2:
      return std::make_shared<UxROM>();
    case 3:
      return std::make_shared<CNROM>();
    case 4:
      return std::make_shared<MMC3>();
    case 7:
      return std::make_shared<AxROM>();
  }
  throw std::runtime_error("Unimplemented mapper");
}

#pragma clang diagnostic pop