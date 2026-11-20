#!/usr/bin/env bash
#
# mayhem/build.sh — build iguana's fuzz harnesses and its doctest suite.
#
# iguana is header-only: the "project build" is the harness TU itself, which instantiates the
# library's readers/writers, so compiling each harness with $SANITIZER_FLAGS instruments the
# fuzzed library code. Harness binaries land in /mayhem/out/ (gitignored by upstream's `out/`).
# The test suite is built with upstream's CMake and its NORMAL flags into build-tests/ (gitignored
# by upstream's `build*`).
set -euo pipefail

[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${SRC:=/mayhem}"
export SANITIZER_FLAGS DEBUG_FLAGS CC CXX LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"

# UBSan's `alignment` check is relaxed: iguana's SWAR scanners (json_util.hpp, xml_util.hpp) load
# uint64_t words from arbitrary byte offsets of the input on EVERY input, which would make every
# execution "crash". Everything else in ASan + UBSan stays on and halting.
FUZZ_CXXFLAGS="-std=c++20 -O1 -I$SRC $SANITIZER_FLAGS -fno-sanitize=alignment $DEBUG_FLAGS"

OUT="$SRC/out"
OBJ="$SRC/out/obj"
mkdir -p "$OUT" "$OBJ"

HARNESSES="fuzz_json fuzz_json_dom fuzz_prettify fuzz_yaml fuzz_xml fuzz_pb"

# Shared objects: the LSan-off hook and the run-once standalone driver (compiled as C so its
# LLVMFuzzerTestOneInput reference keeps C linkage).
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c "$SRC/mayhem/lsan_off.cc" -o "$OBJ/lsan_off.o"
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$OBJ/standalone_main.o"

# 1+2) Each harness: one instrumented object, linked twice (libFuzzer binary + standalone reproducer).
build_one() {
  local h="$1"
  $CXX $FUZZ_CXXFLAGS -fsanitize=fuzzer-no-link -c "$SRC/mayhem/harnesses/$h.cpp" -o "$OBJ/$h.o"
  $CXX $FUZZ_CXXFLAGS $LIB_FUZZING_ENGINE "$OBJ/$h.o" "$OBJ/lsan_off.o" -o "$OUT/$h"
  $CXX $FUZZ_CXXFLAGS "$OBJ/$h.o" "$OBJ/standalone_main.o" "$OBJ/lsan_off.o" -o "$OUT/$h-standalone"
}
pids=()
for h in $HARNESSES; do
  build_one "$h" & pids+=($!)
done
rc=0
for p in "${pids[@]}"; do wait "$p" || rc=1; done
[ "$rc" -eq 0 ] || { echo "harness build failed" >&2; exit 1; }

# 3) Upstream's doctest suite with its NORMAL flags (no sanitizers), via upstream CMake.
TESTS="test_some test_ut test_xml test_xml_nothrow test_yaml test_nothrow test_util test_pb test_cpp20"
cmake -S "$SRC" -B "$SRC/build-tests" \
  -DCMAKE_CXX_FLAGS="$COVERAGE_FLAGS" -DCMAKE_EXE_LINKER_FLAGS="$COVERAGE_FLAGS"
# shellcheck disable=SC2086
cmake --build "$SRC/build-tests" -j"$MAYHEM_JOBS" --target $TESTS
