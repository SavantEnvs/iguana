// Fuzz iguana::from_xml into reflected structs (incl. attributes / CDATA),
// then round-trip via to_xml.
#include <cstdint>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "iguana/xml_reader.hpp"
#include "iguana/xml_writer.hpp"

namespace fz {
enum class level { low, mid, high };
struct child {
  std::string key1;
  int key2;
  std::optional<double> key3;
};
YLT_REFL(child, key1, key2, key3);

struct book {
  iguana::xml_attr_t<std::string> title;
  std::string author;
  std::vector<int> years;
};
YLT_REFL(book, title, author, years);

struct root {
  bool b;
  char c;
  int i;
  uint64_t u;
  float f;
  double d;
  std::string s;
  std::string_view sv;
  level lv;
  std::optional<std::string> os;
  std::unique_ptr<child> up;
  child ch;
  std::vector<child> children;
  std::vector<book> books;
  std::vector<std::string> names;
  iguana::xml_attr_t<child, std::map<std::string_view, std::string_view>> attrc;
  iguana::xml_cdata_t<std::string> cdata;
};
YLT_REFL(root, b, c, i, u, f, d, s, sv, lv, os, up, ch, children, books, names,
         attrc, cdata);

using rooted = iguana::xml_attr_t<root, std::map<std::string, std::string>>;
}  // namespace fz

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string in(reinterpret_cast<const char *>(data), size);
  {
    fz::root r{};
    try {
      iguana::from_xml(r, in);
      std::string out;
      iguana::to_xml(r, out);
    } catch (const std::exception &) {
    }
  }
  {
    fz::rooted r{};
    try {
      iguana::from_xml(r, in);
      std::string out;
      iguana::to_xml(r, out);
    } catch (const std::exception &) {
    }
  }
  return 0;
}
