// Fuzz the JSON DOM parser iguana::parse(jvalue&, ...) and the prettifier.
#include <cstdint>
#include <cstddef>
#include <string>

#include "iguana/json_reader.hpp"
#include "iguana/json_writer.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string in(reinterpret_cast<const char *>(data), size);
  {
    iguana::jvalue v;
    try {
      iguana::parse(v, in.begin(), in.end());
    } catch (const std::exception &) {
    }
  }
  {
    iguana::jvalue v;
    try {
      iguana::parse(v, in.begin(), in.end(), /*parse_as_double=*/true);
    } catch (const std::exception &) {
    }
  }
  {
    iguana::jvalue v;
    std::error_code ec;
    iguana::parse(v, in.begin(), in.end(), ec);
  }
  return 0;
}
