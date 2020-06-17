#include <iostream>

#include "header.hpp"

void inspect_header(NESHeader* h) {
  std::cout << "PRG-ROM size: "
            << prg_rom_size(h) * 0x4000 << std::endl
            << "CHR-ROM size: " << chr_rom_size(h) * 0x2000 << '\n'
            << "Mapper: "
            << mapper_no(h) << '\n'
            << "NES 2.0: "
            << is_nes20_header(h)
            << std::endl;
}
