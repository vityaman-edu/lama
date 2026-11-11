#!/usr/bin/env bash
set -euo pipefail

if [[ $# != 1 || $1 != *.lama || ! -f $1 ]]; then
  echo "Usage: $0 file.lama" >&2
  exit 1
fi

root=$(cd "$(dirname "$0")/.." && pwd)
source_dir=$(cd "$(dirname "$1")" && pwd)
source_file="$source_dir/$(basename "$1")"

printf 'program: |2\n'
sed 's/^/  /' "$source_file"
printf '\n'

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

lamac=("$root/_build/default/src/Driver.exe"
  -runtime "$root/_build/default/runtime"
  -I "$root/_build/default/stdlib/x64" -I "$source_dir")

cd "$tmpdir"
"${lamac[@]}" -b -ds "$source_file" >&2
bytecode="${source_file##*/}"
bytecode="${bytecode%.lama}.bc"

printf 'bytecode_symbols: |2\n'
sed 's/^/  /' "${bytecode%.bc}.sm"
printf '\n'

printf 'virtual_machine_disasm: |2\n'
"$root/_build/default/virtual_machine/lama.exe" --disassemble "$tmpdir/$bytecode" | sed 's/^/  /'
printf '\n'

if [[ -t 0 ]]; then
  echo "Enter program input; press Ctrl-D on an empty line when finished." >&2
fi
cat >"$tmpdir/input"

export LC_ALL=C
TIMEFORMAT=%R
run() {
  local name=$1 status=0
  shift
  { time "$@" <"$tmpdir/input" >"$tmpdir/output" 2>&1; } 2>"$tmpdir/time" || status=$?
  printf '%s:\n  time_seconds: %s\n  exit_code: %s\n  output: |2\n' \
    "$name" "$(cat "$tmpdir/time")" "$status"
  sed 's/^/    /' "$tmpdir/output"
  printf '\n'
}

run interpreter "${lamac[@]}" -i "$source_file"

run virtual_machine "$root/_build/default/virtual_machine/lama.exe" \
  -I "$root/_build/default/stdlib/x64" "$tmpdir/$bytecode"
run virtual_machine_2 "$root/virtual_machine_2/lamavm2" "$tmpdir/$bytecode"
