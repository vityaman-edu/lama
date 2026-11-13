#!/usr/bin/env bash
set -euo pipefail

mode=${1:-rel}
are_traces_enabled=OFF

case "$mode" in
  rel) build_type=Release; are_sanitizers_enabled=OFF ;;
  san) build_type=Debug; are_sanitizers_enabled=ON ;;
  *) echo "Usage: $0 [rel|san] [trace]" >&2; exit 2 ;;
esac

if [[ $# -gt 2 || ( $# -eq 2 && "$2" != trace ) ]]; then
  echo "Usage: $0 [rel|san] [trace]" >&2
  exit 2
fi

if [[ ${2:-} == trace ]]; then
  are_traces_enabled=ON
fi

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
compiler_flags=()

if [[ $(uname -s) == Darwin ]]; then
  unset DEVELOPER_DIR SDKROOT
  compiler_flags=(\
    -DCMAKE_C_COMPILER="$(/usr/bin/xcrun --find clang)" \
    -DCMAKE_CXX_COMPILER="$(/usr/bin/xcrun --find clang++)" \
    -DCMAKE_ASM_COMPILER="$(/usr/bin/xcrun --find clang)" \
    -DCMAKE_OSX_SYSROOT="$(/usr/bin/xcrun --sdk macosx --show-sdk-path)")
fi

cmake -S \
  "$vm_dir" \
  -B "$vm_dir/build/$mode" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE="$build_type" \
  -DLAMAVM_SANITIZERS="$are_sanitizers_enabled" \
  -DLAMAVM_TRACE="$are_traces_enabled" \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  "${compiler_flags[@]}"
