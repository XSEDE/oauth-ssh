define HELPTEXT
Usage: make {develop|test|release|clean}

develop:  configure client and server for development
test:     run all tests
release:  build client and server packages
clean:    remove any built artifacts
endef
export HELPTEXT

help all:
	@echo "$$HELPTEXT"

develop:
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
	$(MAKE) -C server clean

.PHONY: all develop test release clean
