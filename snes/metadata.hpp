#pragma once

#include <vector>

#include "types.h"

struct Metadata {
  enum class Mapper { FastROM, HiROM, LoROM } mapper;
  char name[22] {};
  byte rom {};
  uint32_t rom_size {};
  uint32_t sram_size {};
  byte creator {};
  byte version {};
  byte checksum {};
  byte checksum_comp {};
  word nmi {};
  word rst {};
};

void inspect(const Metadata& meta);

Metadata interpret(const std::vector<byte>& data);