#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <algorithm>
#include <deque>
#include "types.h"
#include "cpu.h"
#include "util.h"

#include <gflags/gflags.h>
#include "rang.hpp"

bool should_dump = false;
bool cloop = false;

DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(cloop, false, "When dumping disassembly, detect and condense loops");
DEFINE_bool(headless, false, "No display");
DEFINE_bool(limit_framerate, true, "Limit framerate to 59.7 fps");
DEFINE_int32(us_per_frame, 17'500, "ms per frame limit");
DEFINE_int32(run_for_n, -1, "Run for n instructions");
DEFINE_int32(run_for_cycles, -1, "Run for n cycles");
DEFINE_string(dump_states_to_file, "", "Dump states to file");
DEFINE_string(dis_instrs, "", "Instructions to dump for");
DEFINE_string(dis_pcs, "", "ROM locations to dump for");
DEFINE_bool(dis_dump_from_pc, false, "Start dumping once pc has gotten to one of the values in dis_pcs");
DEFINE_bool(fake_boot, true, "Initialise registers to post-ROM values");
DEFINE_string(model, "", "Model to emulate");
DEFINE_bool(td, false, "Show tile debugger");
DEFINE_bool(tm, false, "Show tile map");
DEFINE_bool(no_load, false, "Don't load external RAM from file");
DEFINE_bool(no_save, false, "Don't save external RAM to file");
DEFINE_bool(audio, false, "Enable sound");

DEFINE_bool(xx, false, "Debug");

using namespace std;

bool in_title = true;
bool xx = false;

int main(int argc, char** argv) {
  gflags::SetUsageMessage("A Game Boy emulator");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  
  xx = FLAGS_xx;
  
  if (argc != 2) {
    std::cerr << "Expecting a ROM filename." << std::endl;
    exit(1);
  }
  CPU cpu(argv[1]);
  
  std::deque<word> history;
  size_t repeating = 0;
  size_t last_period = 0;
  
  if (FLAGS_fake_boot) {
    cpu.fake_boot();
  } else {
    while (cpu.pc != 0x100) {
      cpu.step(false);
    }
  }
  
  cpu.mmu.dump_cartridge_info();
  
  in_title = false;
  
  should_dump = FLAGS_dis;
  cloop = FLAGS_cloop;
  
  int run_for_n = FLAGS_run_for_n;
  int run_for_cycles = FLAGS_run_for_cycles;
  
  std::ofstream states_file;
  if (FLAGS_dump_states_to_file != "") {
    states_file.open(FLAGS_dump_states_to_file);
  }
  
  if (!FLAGS_headless)
    cpu.ppu->screen->on();
  
  long ninstrs = 0;
  while ((run_for_n == -1 || ninstrs < run_for_n) &&
         (run_for_cycles == -1 || cpu.cycles < run_for_cycles)) {
    if (cloop) {
      history.emplace_back(cpu.pc);
      if (history.size() >= 20) {
        history.pop_front();
      }
      
      auto period = history_repeating(history);
      if (period) {
        repeating++;
        should_dump = (repeating == 1);
      } else {
        if (repeating > 0) {
          std::cout << "\t... [last " << dec << last_period << " ops repeated " << dec << repeating << " times]" << std::endl << std::endl;
        }
        
        repeating = 0;
        should_dump = true;
      }
      last_period = period;
    }
  
    cpu.step(should_dump);
    if (FLAGS_dump_states_to_file != "") {
      cpu.dump_registers_to_file(states_file);
    }
    
    ++ninstrs;
  }
}
