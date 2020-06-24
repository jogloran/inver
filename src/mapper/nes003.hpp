#pragma once

#include <cereal/types/polymorphic.hpp>
#include <cereal/archives/binary.hpp>

#include "mapper.hpp"

class CNROM : public Mapper {
public:
  CNROM(): chr_bank(0), mirroring(Mapper::Mirroring::Unknown) {}

  Mirroring get_mirroring() override;

  void
  map(const std::vector<char>& data, byte prg_banks, byte chr_banks, const NESHeader* header) override;

  byte chr_read(word addr) override;

  void chr_write(word addr, byte value) override;

  void reset() override;

  std::vector<byte> rom;
  std::vector<byte> chr;
  byte chr_bank;

  Mapper::Mirroring mirroring;

  byte* bank(byte bank);

  byte read(word addr) override;

  void write(word addr, byte value) override;

  template<typename Ar>
  void serialize(Ar& ar) {
    ar(rom, chr, chr_bank, mirroring);
  }

  friend class cereal::access;
};

CEREAL_REGISTER_TYPE(CNROM)
CEREAL_REGISTER_POLYMORPHIC_RELATION(Mapper, CNROM)