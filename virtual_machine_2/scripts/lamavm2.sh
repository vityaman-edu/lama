#!/usr/bin/env bash
set -euo pipefail

mode=${1:-rel}

case "$mode" in
  rel) build_type=Release; are_sanitizers_enabled=OFF ;;
  san) build_type=Debug; are_sanitizers_enabled=ON ;;
  *) echo "Usage: $0 [rel|san]" >&2; exit 2 ;;
esac

if [[ $# -gt 1 ]]; then
  echo "Usage: $0 [rel|san]" >&2
  exit 2
fi

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
compiler_flags=()

if [[ $(uname -s) == Darwin ]]; then
  unset DEVELOPER_DIR SDKROOT
  compiler_flags=(-DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ \
    -DCMAKE_OSX_SYSROOT="$(/usr/bin/xcrun --sdk macosx --show-sdk-path)")
fi

cmake -S "$vm_dir" -B "$vm_dir/build/$mode" -G Ninja \
  -DCMAKE_BUILD_TYPE="$build_type" -DLAMAVM_SANITIZERS="$are_sanitizers_enabled" \
  -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  "${compiler_flags[@]}"
cmake --build "$vm_dir/build/$mode" --target lamavm2 --parallel "${LAMAVM_BUILD_JOBS:-2}"
ln -sfn "build/$mode/lamavm2" "$vm_dir/lamavm2"
