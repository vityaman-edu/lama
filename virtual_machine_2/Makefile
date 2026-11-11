.PHONY: lamavm2
lamavm2:
	printf '%s\n' '#!/bin/sh' 'echo todo' > lamavm2
	chmod +x lamavm2

.PHONY: style
style:
	echo todo

.PHONY: check
check:
	echo todo

.PHONY: test
test: check
	echo todo

.PHONY: lamac
lamac:
	(cd ..; dune build src/Driver.exe)
	rm -f lamac
	ln -s ../_build/default/src/Driver.exe lamac

.PHONY: lamavm
lamavm:
	(cd ..; dune build virtual_machine/lama.exe)
	rm -f lamavm
	printf '%s\n' '#!/bin/sh' '"$(abspath ../virtual_machine/lama.exe)" -I "$(abspath ../stdlib/x64)" "$$@"' > lamavm
	chmod +x lamavm
