.PHONY: lamavm2 lamavm2-san style style-cpp style-py check check-cpp check-py test test-san lamac lamavm

lamavm2:
	./scripts/lamavm2.sh

lamavm2-san:
	./scripts/lamavm2.sh san

style: style-cpp style-py

style-cpp:
	./scripts/style-cpp.sh

style-py:
	./scripts/style-py.sh

check: check-cpp check-py

check-cpp:
	./scripts/check-cpp.sh

check-py:
	./scripts/check-py.sh

test: check lamac lamavm lamavm2
	./scripts/test.sh

test-san: check lamac lamavm lamavm2-san
	./scripts/test.sh san

lamac:
	./scripts/lamac.sh

lamavm:
	./scripts/lamavm.sh
