#include "debug.hpp"

std::set<std::string> active_tags;
std::map<dword, PCWatchSpec> dis_pcs;
std::set<dword> ignored_pcs;
std::vector<ChangeWatchSpec> change_watches;

std::vector<ChangeWatchSpec> parse_change_watches(std::string change_watches_str) {
  auto tags = parse_tags(change_watches_str);
  std::vector<ChangeWatchSpec> result;
  std::transform(tags.begin(), tags.end(), std::back_inserter(result),
                 [](std::string tag) {
                   unsigned long loc = tag.find("=");
                   auto watch_name = tag.substr(0, loc);
                   dword addr = std::stoi(tag.substr(loc + 1), 0, 16);
                   return ChangeWatchSpec { watch_name, addr };
                 });
  return result;
}

std::map<dword, PCWatchSpec> parse_pc_watch_spec(std::string dis_pcs_str) {
  auto tags = parse_tags(dis_pcs_str);
  std::map<dword, PCWatchSpec> result;
  std::transform(tags.begin(), tags.end(), std::inserter(result, result.end()),
                 [](std::string tag) {
                   unsigned long pos = tag.rfind(":");
                   PCWatchSpec::Action action;
                   if (pos == std::string::npos) {
                     action = PCWatchSpec::All;
                   } else {
                     auto rwx = tag.substr(pos + 1);
                     action = watch_spec_interpret_rwx(rwx);
                   }

                   bool log_from_here = tag[tag.size() - 1] == '+';
                   dword addr = std::stoi(tag, 0, 16);
                   return std::make_pair(addr, PCWatchSpec { addr, action, log_from_here });
                 });
  return result;
}

std::set<dword> parse_ignored_pcs(std::string ignored_pcs_str) {
  auto tags = parse_tags(ignored_pcs_str);
  std::set<dword> result;
  std::transform(tags.begin(), tags.end(), std::inserter(result, result.end()),
                 [](std::string tag) {
                   return std::stoi(tag, 0, 16);
                 });
  return result;
}

std::set<std::string> parse_tags(std::string tags_str) {
  std::set<std::string> result;
  if (tags_str == "") return result;

  std::string tags(tags_str);
  std::replace(tags.begin(), tags.end(), ',', ' ');

  std::istringstream ss(tags);
  std::copy(std::istream_iterator<std::string>(ss),
            std::istream_iterator<std::string>(),
            std::inserter(result, result.begin()));

  return result;
}
