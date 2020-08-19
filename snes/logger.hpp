#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <set>
#include <string>

extern std::set<std::string> active_tags;

template <typename T>
class Logger {
public:
  void log_with_tag(const char* tag, const char* msg, ...) {
    va_list args;
    va_start(args, msg);
    log_with_tag(tag, msg, args);
    va_end(args);
  }

  void log_with_tag(const char* tag, const char* msg, va_list args) {
    if (std::none_of(active_tags.begin(), active_tags.end(), [tag](const auto& active_tag) {
      return active_tag.rfind(tag, 0) == 0;
    })) {
      return;
    }

    static char buf[1024];
    std::snprintf(buf, 1024, "%17.17s | ", tag);
    std::strcat(buf, msg);
    std::vprintf(buf, args);
  }
  void log(const char* msg, ...) {
    if (active_tags.find(T::TAG) == active_tags.end()) return;

    va_list args;
    static char buf[1024];
    std::snprintf(buf, 1024, "%17.17s | ", T::TAG);
    std::strcat(buf, msg);
    va_start(args, msg);
    std::vprintf(buf, args);
    va_end(args);
  }
};