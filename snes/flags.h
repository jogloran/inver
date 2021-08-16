#include <gflags/gflags.h>

DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(dis_raw, false, "Don't output ansi colours in disassembly");
DEFINE_bool(xx, false, "Debug");
DEFINE_bool(dump_stack, false, "Dump stack");
DEFINE_bool(audio, false, "Enable audio");
DEFINE_string(tags, "", "Log tags to print, separated by commas");
DEFINE_bool(td, false, "Show tile debugger (nametable)");
DEFINE_bool(show_raster, false, "Show raster");
DEFINE_string(dis_pcs, "", "ROM locations to dump for");
DEFINE_string(ignored_pcs, "", "ROM locations to not dump for");
DEFINE_string(change_watches, "", "Memory locations to watch for changes at");
DEFINE_bool(fake_sprites, false, "Displays a red rectangle at the top left of each sprite");
DEFINE_int32(sram, 0x10000, "Size of the first SRAM bank (70-7d:0000-ffff)");
DEFINE_bool(load_save, false, "Load save immediately");
DEFINE_bool(show_main, false, "");
DEFINE_bool(show_sub, false, "");
DEFINE_bool(m7, false, "Show mode 7 visualisation");
DEFINE_bool(dis_dump_m7, false, "Show m7 params when dumping disassembly");
