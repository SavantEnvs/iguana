// Fuzz the JSON prettifier state machine (iguana/prettify.hpp).
#include <cstdint>
#include <cstddef>
#include <string>

#include "iguana/prettify.hpp"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::string in(reinterpret_cast<const char *>(data), size);
  std::string a = iguana::prettify(in);
  std::string b = iguana::prettify(in, /*tabs=*/true, 1);
  (void)a;
  (void)b;
  return 0;
}
