//
// Created by Daniel Tse on 24/5/20.
//

#include "nes004.h"

Mapper::Mirroring MMC3::get_mirroring() {
  return Mirroring::Unknown;
}

void MMC3::irq_enable(bool enable) {

}
