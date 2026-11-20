// Fuzz iguana::from_json into a wide reflected struct, then round-trip via to_json.
#include <cstdint>
#include <cstddef>
#include <array>
#include <map>
#include <unordered_map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <deque>
#include <list>

#include "iguana/json_reader.hpp"
#include "iguana/json_writer.hpp"

namespace fz {
enum class color { red, green, blue };

struct point {
  int x;
  double y;
};
YLT_REFL(point, x, y);

struct inner {
  std::string name;
  std::optional<int> id;
  std::vector<point> pts;
  std::map<std::string, int> counts;
};
YLT_REFL(inner, name, id, pts, counts);

struct wide {
  bool b;
  char c;
  int8_t i8;
  uint8_t u8;
  int16_t i16;
  uint16_t u16;
  int32_t i32;
  uint32_t u32;
  int64_t i64;
  uint64_t u64;
  float f;
  double d;
  std::string s;
  std::string_view sv;
  color col;
  std::array<int, 3> arr;
  std::array<char, 4> carr;
  std::vector<int> vi;
  std::vector<std::string> vs;
  std::deque<double> dd;
  std::list<bool> lb;
  std::vector<inner> vin;
  std::map<std::string, inner> min;
  std::unordered_map<int, std::string> umap;
  std::optional<std::string> os;
  std::unique_ptr<point> up;
  std::shared_ptr<inner> sp;
  std::variant<int, std::string, double> var;
  inner nested;
  std::vector<std::vector<int>> vvi;
};
YLT_REFL(wide, b, c, i8, u8, i16, u16, i32, u32, i64, u64, f, d, s, sv, col,
         arr, carr, vi, vs, dd, lb, vin, min, umap, os, up, sp, var, nested,
         vvi);
}  // namespace fz

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Keep the input alive for the whole iteration: string_view fields alias it.
  std::string in(reinterpret_cast<const char *>(data), size);
  {
    fz::wide w{};
    try {
      iguana::from_json(w, in);
      std::string out;
      iguana::to_json(w, out);
    } catch (const std::exception &) {
    }
  }
  {
    fz::wide w{};
    std::error_code ec;
    iguana::from_json(w, in, ec);
  }
  {
    std::vector<fz::point> v;
    try {
      iguana::from_json(v, in);
    } catch (const std::exception &) {
    }
  }
  return 0;
}
