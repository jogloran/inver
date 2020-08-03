#pragma once

#include <cstdio>
#include <string>
#include <memory>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <fstream>
#include <sstream>
#include <set>
#include <gflags/gflags.h>
#include <map>

#include "types.h"
#include "rang.hpp"
#include "cpu5a22.hpp"
#include "bus_snes.hpp"

struct ChangeWatchSpec {
  std::string name;
  dword addr;
  byte last_value = 0xff;

  void operator()(byte val) {
    if (val != last_value) {
      last_value = val;
      std::printf("%s: %02x\n", name.c_str(), last_value);
    }
  }
};

struct PCWatchSpec {
  dword addr;
  enum Action {
    None = 0,
    R = 1 << 0,
    W = 1 << 1,
    X = 1 << 2,
    All = 0b111,
  } action;
  bool log_from_here = false;

  bool operator<(PCWatchSpec& other) {
    return addr < other.addr;
  }
};

inline PCWatchSpec::Action operator|(PCWatchSpec::Action a, PCWatchSpec::Action b) {
  using T = std::underlying_type<PCWatchSpec::Action>::type;
  return static_cast<PCWatchSpec::Action>(static_cast<T>(a) | static_cast<T>(b));
}

inline PCWatchSpec::Action operator|=(PCWatchSpec::Action& a, PCWatchSpec::Action b) {
  return a = a | b;
}

static PCWatchSpec::Action watch_spec_interpret_rwx(std::string rwx) {
  PCWatchSpec::Action a = PCWatchSpec::None;
  for (int i = 0; i < rwx.size(); ++i) {
    switch (rwx[i]) {
      case 'r': a |= PCWatchSpec::Action::R; break;
      case 'w': a |= PCWatchSpec::Action::W; break;
      case 'x': a |= PCWatchSpec::Action::X; break;
    }
  }
  return a;
}

std::set<std::string> parse_tags(std::string tags_str);

std::set<dword> parse_ignored_pcs(std::string ignored_pcs_str);

std::map<dword, PCWatchSpec> parse_pc_watch_spec(std::string dis_pcs_str);

std::vector<ChangeWatchSpec> parse_change_watches(std::string change_watches_str);
