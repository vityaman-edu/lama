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

lamavm2:
	echo todo

style:
	echo todo

lint:
	echo todo

test:
	echo todo
