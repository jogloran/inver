#include <iostream>

#include <gflags/gflags.h>
#include <zip.h>
#include <fstream>

#include "header.hpp"
#include "fort.hpp"

DEFINE_string(path, "", "");
DEFINE_int32(unzip, -1, "");
DEFINE_int32(mapper, -1, "");
DEFINE_string(out, "out.nes", "");
DEFINE_bool(summary, false, "");
DEFINE_int32(summary_limit, 15, "");

class Zip {
public:
  class iterator {
  public:
    struct entry {
      zip_file_t* f;
      std::string name;
      zip_uint64_t i;

      ~entry() {
        if (f != nullptr) zip_fclose(f);
      }
    };

    iterator(Zip& z, const zip_uint64_t p = 0) : zip(z), pos(p) {}

    bool operator!=(const iterator& other) const { return pos != other.pos; }

    entry operator*() const {
      return {zip_fopen_index(zip.zip, pos, 0), zip_get_name(zip.zip, pos, 0), pos};
    }

    const iterator& operator++() {
      ++pos;
      return *this;
    }

  private:
    Zip& zip;
    zip_uint64_t pos;
  };

  Zip(std::string p) : path(p) {
    int err;
    zip = zip_open(p.c_str(), ZIP_RDONLY, &err);
  }

  ~Zip() {
    zip_close(zip);
  }

  operator bool() {
    return zip != nullptr;
  }

  iterator::entry entry_at(zip_uint64_t i);

  void unpack(iterator::entry entry, std::ostream& ofstream);

private:
  std::string path;
  zip_t* zip;

  friend Zip::iterator begin(Zip& z);

  friend Zip::iterator end(Zip& z);
};

Zip::iterator begin(Zip& z) {
  return Zip::iterator(z);
}

Zip::iterator end(Zip& z) {
  return Zip::iterator(z, zip_get_num_entries(z.zip, 0));
}

Zip::iterator::entry Zip::entry_at(zip_uint64_t pos) {
  return {zip_fopen_index(zip, pos, 0), zip_get_name(zip, pos, 0), pos};
}

void Zip::unpack(Zip::iterator::entry entry, std::ostream& out) {
  static char buf[4096] = {};
  zip_int64_t nbytes;
  while ((nbytes = zip_fread(entry.f, (void*) buf, 4096)) > 0) {
    out.write(buf, nbytes);
  }
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage("Rhomb");
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  static char buf[17] = {};

  Zip z {FLAGS_path};
  if (!z) {
    std::cerr << "Couldn't open zip file: '" << FLAGS_path << '\'' << std::endl;
    std::exit(1);
  }

  if (FLAGS_unzip != -1) {
    auto entry = z.entry_at(FLAGS_unzip);
    {
      std::ofstream out(FLAGS_out);
      z.unpack(entry, out);
    }
    std::exit(0);
  }

  if (FLAGS_summary) {
    std::map<int, size_t> counts;
    for (Zip::iterator::entry f : z) {
      if (f.name.find("/nes/") == std::string::npos || f.name.rfind(".nes") == std::string::npos)
        continue;

      NESHeader h;
      auto bytes_read = zip_fread(f.f, (void*) &h, 0x10);
      if (bytes_read == 0x10) {
        if (!is_valid_header(&h)) continue;

        ++counts[mapper_no(&h)];
      }
    }

    fort::char_table tb;
    tb << fort::header << "Mapper" << "Count" << fort::endr;
    tb.column(0).set_cell_text_align(fort::text_align::right);
    tb.column(1).set_cell_text_align(fort::text_align::right);

    std::multimap<size_t, int> by_counts;
    for (const auto& [mapper, count] : counts) by_counts.emplace(count, mapper);

    for (auto it = by_counts.rbegin(); it != by_counts.rend(); ++it) {
      tb << it->second << it->first << fort::endr;
      if (--FLAGS_summary_limit == 0) {
        tb << "..." << "..." << fort::endr;
        break;
      }
    }

    std::cout << tb.to_string() << std::endl;
    std::exit(0);
  }

  fort::char_table tb;
//  tb.set_border_style(FT_DOUBLE2_STYLE);
  tb.column(2).set_cell_text_align(fort::text_align::right);
  tb.column(3).set_cell_text_align(fort::text_align::right);
  tb.column(4).set_cell_text_align(fort::text_align::right);
  tb << fort::header
     << "#" << "File" << "Mapper" << "PRG" << "CHR" << fort::endr;

  for (Zip::iterator::entry f : z) {
    if (f.name.find("/nes/") == std::string::npos || f.name.rfind(".nes") == std::string::npos)
      continue;

    NESHeader h;
    auto bytes_read = zip_fread(f.f, (void*) &h, 0x10);
    if (bytes_read == 0x10) {
      if (!is_valid_header(&h)) continue;

      if (size_t pos = f.name.rfind('/'); pos != std::string::npos) {
        std::string fn = f.name.substr(pos + 1);
        if (FLAGS_mapper == -1 || mapper_no(&h) == FLAGS_mapper) {
          tb << f.i
             << fn << mapper_no(&h) << prg_rom_size(&h) * 0x4000 / 1024
             << chr_rom_size(&h) * 0x2000 / 1024
             << fort::endr;
        }
      }
    }
  }

  std::cout << tb.to_string() << std::endl;
}