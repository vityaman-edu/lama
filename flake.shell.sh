#!/usr/bin/env bash

if [ ! -f "${OPAMROOT:-$HOME/.opam}/config" ]; then
  opam init --bare --no-setup --yes || exit 1
fi

if ! opam switch list --short | grep -qx lama-dev; then
  opam switch create lama-dev ocaml-system --yes || exit 1
fi

eval "$(opam env --switch=lama-dev --set-switch)"

CAML_LD_LIBRARY_PATH="$(opam var lib)/stublibs${CAML_LD_LIBRARY_PATH:+:$CAML_LD_LIBRARY_PATH}"
export CAML_LD_LIBRARY_PATH

opam install . --deps-only --assume-depexts --yes || exit 1

export LAMA_REPO="$PWD"

lamac() {
  "$LAMA_REPO/_build/default/src/Driver.exe" "$@"
}
