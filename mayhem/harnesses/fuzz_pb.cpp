// Fuzz iguana::from_pb (protobuf wire format) into reflected messages, then
// round-trip via to_pb.
#include <cstdint>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "iguana/pb_reader.hpp"
#include "iguana/pb_writer.hpp"

namespace fz {
enum class color : int32_t { red = 0, green = 1, blue = 2 };

struct leaf {
  int32_t a;
  std::string s;
};
YLT_REFL(leaf, a, s);

struct msg {
  int32_t i32;
  int64_t i64;
  uint32_t u32;
  uint64_t u64;
  iguana::sint32_t s32;
  iguana::sint64_t s64;
  iguana::fixed32_t f32;
  iguana::fixed64_t f64;
  iguana::sfixed32_t sf32;
  iguana::sfixed64_t sf64;
  float fl;
  double db;
  bool b;
  std::string str;
  color col;
  std::vector<int32_t> vi;
  std::vector<double> vd;
  std::vector<std::string> vs;
  std::vector<leaf> vl;
  std::map<int32_t, std::string> mis;
  std::map<std::string, leaf> msl;
  leaf nested;
  std::optional<leaf> ol;
  std::optional<int32_t> oi;
  std::variant<std::monostate, int32_t, std::string, leaf> one;
};
YLT_REFL(msg, i32, i64, u32, u64, s32, s64, f32, f64, sf32, sf64, fl, db, b,
         str, col, vi, vd, vs, vl, mis, msl, nested, ol, oi, one);
}  // namespace fz

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string in(reinterpret_cast<const char *>(data), size);
  fz::msg m{};
  try {
    iguana::from_pb(m, std::string_view(in));
    std::string out;
    iguana::to_pb(m, out);
    fz::msg m2{};
    iguana::from_pb(m2, std::string_view(out));
  } catch (const std::exception &) {
  }
  return 0;
}
