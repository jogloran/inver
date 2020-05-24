#include "mbc_types.h"
#include "mmu.h"

#include "mbc1.h"
#include "mbc2.h"
#include "mbc3.h"
#include "mbc5.h"

std::ostream& operator<<(std::ostream& out, MBC mbc) {
  if (mbc_string.find(mbc) != mbc_string.end()) {
    out << mbc_string[mbc];
  } else {
    out << "(unknown)";
  }
  
  return out;
}

std::unique_ptr<MBCBase> mbc_for(MBC mbc, MMU& mmu) {
  switch (mbc) {
    case MBC::ROM:
    case MBC::ROM_RAM:
    case MBC::ROM_RAM_BATTERY:
      //    return std::make_unique<ROM>();
      
    case MBC::MBC1:
    case MBC::MBC1_RAM:
    case MBC::MBC1_RAM_BATTERY:
      return std::make_unique<MBC1>(mmu);
      
    case MBC::MBC3:
    case MBC::MBC3_TIMER_BATTERY:
    case MBC::MBC3_TIMER_RAM_BATTERY:
    case MBC::MBC3_RAM:
    case MBC::MBC3_RAM_BATTERY:
      return std::make_unique<MBC3>(mmu);
      
    case MBC::MBC2:
    case MBC::MBC2_BATTERY:
      return std::make_unique<MBC2>(mmu);
    
    case MBC::MBC5:
    case MBC::MBC5_RAM:
    case MBC::MBC5_RUMBLE:
    case MBC::MBC5_RUMBLE_RAM:
    case MBC::MBC5_RAM_BATTERY:
    case MBC::MBC5_RUMBLE_RAM_BATTERY:
      return std::make_unique<MBC5>(mmu);
      
    default:
      throw std::runtime_error(std::string("Unsupported MBC: ") + mbc_string[mbc]);
  }
}
