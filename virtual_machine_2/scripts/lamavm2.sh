#!/usr/bin/env bash
set -euo pipefail

mode=${1:-rel}

if [[ $(uname -s) == Darwin ]]; then
  unset DEVELOPER_DIR SDKROOT
fi

vm_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
"$vm_dir/scripts/configure.sh" "$@"
cmake --build "$vm_dir/build/$mode" --target lamavm2 --parallel "${LAMAVM_BUILD_JOBS:-2}"
ln -sfn "build/$mode/lamavm2" "$vm_dir/lamavm2"
