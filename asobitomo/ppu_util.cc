#include "ppu_util.h"

#include <iomanip>
#include <iostream>

using std::hex;
using std::setw;

using namespace std;

void
compare_oams(OAM* current_oam, OAM* old_oam) {
  bool first = true;
  
  std::stringstream s;
  for (int i = 0; i < 40; ++i) {
    OAM cur = current_oam[i];
    OAM old = old_oam[i];
    
    if (cur != old) {
      if (first) {
        first = false;
      } else {
        s << ", ";
      }
      s << "obj " << hex << setw(2) << setfill('0') << i << " [";
      
      bool first_change = true;
      
      if (cur.x != old.x) {
        if (first_change) first_change = false;
        else s << ", ";
        s << "x " << dec << static_cast<int>(old.x) << " -> " << static_cast<int>(cur.x) << hex;
      }
      if (cur.y != old.y) {
        if (first_change) first_change = false;
        else s << ", ";
        s << "y " << dec << static_cast<int>(old.y) << " -> " << static_cast<int>(cur.y) << hex;
      }
      if (cur.tile_index != old.tile_index) {
        if (first_change) first_change = false;
        else s << ", ";
        s << "t " << static_cast<int>(old.tile_index) << " -> " << static_cast<int>(cur.tile_index);
      }
      if (cur.flags != old.flags) {
        if (first_change) first_change = false;
        else s << ", ";
        s << "f " << static_cast<int>(old.flags) << " -> " << static_cast<int>(cur.flags);
      }
      
      s << "]";
    }
  }
  
  std::string out = s.str();
  if (out.size()) {
    std::cout << s.str() << std::endl;
  }
}

std::ostream& operator<<(std::ostream& out, const OAM& oam) {
  return out << "OAM(" << hex << static_cast<int>(oam.x) << ", "
    << static_cast<int>(oam.y)
    << ", tile_index = " << setw(2) << hex << static_cast<int>(oam.tile_index)
    << ", flags = " << binary(oam.flags) << ")";
}
