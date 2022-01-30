define HELPTEXT
Usage: make {develop|test|release|lint|clean}

develop:  configure client and server for development
test:     run all tests
release:  build client and server packages
lint:     run formatters and linters
clean:    remove any built artifacts
endef
export HELPTEXT

help all:
	@echo "$$HELPTEXT"

develop: _install-pre-commit
	$(MAKE) -C client develop
	$(MAKE) -C server -f Makefile.bootstrap develop

test:
	$(MAKE) -C client test
	$(MAKE) -C server -f Makefile.bootstrap test

release:
	$(MAKE) -C client release
	$(MAKE) -C server -f Makefile.bootstrap release

clean:
	$(MAKE) -C client clean
	[[ ! -f server/Makefile ]] || $(MAKE) -C server clean

_venv:
	[[ -d venv ]] || python3 -mvenv venv

_install-pre-commit: _venv
	[[ -f venv/bin/pre-commit ]] || ./venv/bin/pip3 install pre-commit
	[[ -f .git/hooks/pre-commit ]] || ./venv/bin/pre-commit install

lint: _install-pre-commit
	./venv/bin/pre-commit run --all-files

.PHONY: all develop test release clean
