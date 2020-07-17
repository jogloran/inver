#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <set>

extern std::set<std::string> active_tags;

template <typename T>
class Logger {
public:
  void log(const char* msg, ...) {
    if (active_tags.find(T::TAG) == active_tags.end()) return;

    va_list args;
    static char buf[1024];
    std::sprintf(buf, "%17.17s | ", T::TAG);
    std::strcat(buf, msg);
    va_start(args, msg);
    std::vprintf(buf, args);
    va_end(args);
  }
};
