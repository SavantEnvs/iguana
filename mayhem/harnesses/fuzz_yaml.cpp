// Fuzz iguana::from_yaml into a reflected struct, then round-trip via to_yaml.
#include <cstdint>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "iguana/yaml_reader.hpp"
#include "iguana/yaml_writer.hpp"

namespace fz {
enum class kind { a, b, c };
struct item {
  std::string name;
  int count;
  std::vector<std::string> tags;
};
YLT_REFL(item, name, count, tags);

struct sub {
  double ratio;
  std::optional<std::string> note;
  std::map<std::string, int> m;
};
YLT_REFL(sub, ratio, note, m);

struct doc {
  bool flag;
  char ch;
  int i;
  int64_t big;
  uint32_t u;
  float f;
  double d;
  std::string s;
  std::string_view sv;
  kind k;
  std::vector<item> items;
  std::vector<std::string> vs;
  std::vector<int> vi;
  std::map<std::string, int> m;
  std::map<std::string, sub> ms;
  std::optional<std::string> os;
  std::optional<sub> osub;
  std::unique_ptr<item> up;
  sub nested;
};
YLT_REFL(doc, flag, ch, i, big, u, f, d, s, sv, k, items, vs, vi, m, ms, os,
         osub, up, nested);
}  // namespace fz

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string in(reinterpret_cast<const char *>(data), size);
  {
    fz::doc d{};
    try {
      iguana::from_yaml(d, in);
      std::string out;
      iguana::to_yaml(d, out);
    } catch (const std::exception &) {
    }
  }
  {
    fz::doc d{};
    std::error_code ec;
    iguana::from_yaml(d, in, ec);
  }
  {
    std::vector<fz::item> v;
    try {
      iguana::from_yaml(v, in);
    } catch (const std::exception &) {
    }
  }
  return 0;
}
