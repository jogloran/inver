#include <iostream>

#include "header.hpp"

void inspect_header(NESHeader* h) {
  std::cout << "PRG-ROM size: "
            << prg_rom_size(h) * 0x4000 << std::endl
            << "CHR-ROM size: " << chr_rom_size(h) * 0x2000
            << std::endl
            << "Mapper: "
            << mapper_no(h)
            << std::endl;
}
