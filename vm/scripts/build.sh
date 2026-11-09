#!/usr/bin/env bash

set -euo pipefail

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
mode=${1:-release}
sanitizer=none
build_type=Release
fuzzing=OFF

if [ "$#" -gt 1 ]; then
  printf 'Usage: %s [release|asan|ubsan|msan|fuzz]\n' "$0" >&2
  exit 2
fi

case "$mode" in
  release) ;;
  asan) sanitizer=address; build_type=Debug ;;
  ubsan) sanitizer=undefined; build_type=Debug ;;
  msan) sanitizer=memory; build_type=Debug ;;
  fuzz) sanitizer=address; build_type=RelWithDebInfo; fuzzing=ON ;;
  *) printf 'Usage: %s [release|asan|ubsan|msan|fuzz]\n' "$0" >&2; exit 2 ;;
esac

cd "$vm_dir/.."
dune build src/Driver.exe

cmake \
  -S "$vm_dir" \
  -B "$vm_dir/build/$mode" \
  -DCMAKE_BUILD_TYPE="$build_type" \
  -DLAMAVM_SANITIZER="$sanitizer" \
  -DLAMAVM_FUZZ_DURATION="${LAMAVM_FUZZ_DURATION:-10s}" \
  -DFUZZTEST_FUZZING_MODE="$fuzzing"

cmake \
  --build "$vm_dir/build/$mode" \
  --target lamavm lamavm_tests \
  --parallel "${LAMAVM_BUILD_JOBS:-2}"
