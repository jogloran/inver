#pragma once

#include <string>
#include <deque>
#include <iostream>
#include <ios>
#include <iomanip>
#include <algorithm>
#include "types.h"

#include "rang.hpp"

std::string binary(byte b);
std::string interrupt_flags_to_description(byte flags);

std::string replace_path_extension(const std::string path, const std::string old_extension, const std::string extension);

const char* to_flag_string(byte f);

size_t history_repeating(std::deque<word> history);

struct two_byte_fmt_manip {
public:
  two_byte_fmt_manip(byte b1, byte b2): b1_(b1), b2_(b2) {}
  void operator()(std::ostream& out) const {
    out << std::dec << rang::style::bold << rang::fgB::gray << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(b1_) << rang::style::reset << rang::fg::reset
        << std::dec <<                      rang::fgB::black << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(b2_) << rang::style::reset << rang::fg::reset;
  }
  
private:
  byte b1_, b2_;
};

two_byte_fmt_manip two_byte_fmt(byte b1, byte b2);
std::ostream& operator<<(std::ostream& out, const two_byte_fmt_manip& manip);

template <typename T>
void pv(T begin, T end) {
  copy(begin, end, std::ostream_iterator<int>(std::cerr, " "));
  std::cerr << std::endl;
}
