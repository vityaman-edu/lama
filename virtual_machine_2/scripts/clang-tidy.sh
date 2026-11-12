#!/usr/bin/env bash
set -euo pipefail

tool=clang-tidy

if [[ $(uname -s) == Darwin ]]; then
  unset DEVELOPER_DIR SDKROOT

  if command -v clang-tidy-unwrapped >/dev/null 2>&1; then
    tool=clang-tidy-unwrapped
  fi
fi

exec "$tool" "$@"
