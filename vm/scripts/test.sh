#!/usr/bin/env bash

# A non-recursive alias excludes the C VM tests in regression/vm.
exec dune build @@regression/runtest
