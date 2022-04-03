all:
	$(error Usage: "make {develop|test|release} [PYTHON_VERSION=<x.y.z>]")

ifdef PYTHON_VERSION
    PYENV := $(shell command -v pyenv 2> /dev/null)

    ifndef PYENV
        $(error pyenv and pyenv-virtualenv are needed when specifying a python version)
    endif

    USING_PENV=1

    PYENV_ROOT=$(shell pyenv root)
    CREATE_VIRTUAL_ENV=pyenv virtualenv $(PYTHON_VERSION)
    VENV_ROOT=$(PYENV_ROOT)/versions/venv_$(PYTHON_VERSION)
    PY_VER_ROOT=$(PYENV_ROOT)/versions/$(PYTHON_VERSION)
    SETUP_PY_ENV=PYENV_VERSION=$(VENV_NAME) pyenv exec
    PYTHON_EXE=python
    PIP=$(SETUP_PY_ENV) pip
endif

ifndef PYTHON_VERSION
    PYTHON_VERSION=system
    CREATE_VIRTUAL_ENV=virtualenv --python=python3
    VENV_ROOT=venv_$(PYTHON_VERSION)
    PYTHON_EXE=$(VENV_ROOT)/bin/python
    PIP=$(VENV_ROOT)/bin/pip
endif

OAUTH_SSH=$(VENV_ROOT)/bin/oauth-ssh
PYTEST=$(VENV_ROOT)/bin/pytest
BLACK=$(VENV_ROOT)/bin/black
FLAKE8=$(VENV_ROOT)/bin/flake8
VENV_NAME=$(shell basename $(VENV_ROOT))

ifdef USING_PENV
$(PY_VER_ROOT):
	pyenv install --skip-existing $(PYTHON_VERSION)
endif

develop: $(OAUTH_SSH) $(PYTEST) lint_tools
	@echo "Use \". $(VENV_ROOT)/bin/activate\""

lint_tools:
	# These need to be moved when the build system is improved.
	@[[ -f $(VENV_ROOT)/bin/black ]] || $(PIP) install black
	@[[ -f $(VENV_ROOT)/bin/flake8 ]] || $(PIP) install flake8
	@[[ -f $(VENV_ROOT)/bin/mypy ]] || $(PIP) install mypy
	$(PIP) install types-paramiko types-requests types-click

lint: $(OAUTH_SSH) lint_tools
	@-$(VENV_ROOT)/bin/black oauth_ssh/
	@-$(VENV_ROOT)/bin/flake8 oauth_ssh/
	@-$(VENV_ROOT)/bin/mypy oauth_ssh/

test: $(OAUTH_SSH) $(PYTEST)
	$(PYTEST)

$(VENV_ROOT): $(PY_VER_ROOT)
	$(CREATE_VIRTUAL_ENV) $(VENV_NAME)
	$(PIP) install --upgrade pip
	$(PIP) install --upgrade setuptools

$(OAUTH_SSH): $(VENV_ROOT)
	$(SETUP_PY_ENV) $(PYTHON_EXE) setup.py develop

$(PYTEST): $(VENV_ROOT)
	$(PIP) install pytest


#
# Prevent users from building packages from source that was modified after a
# tagged release. If we detect this is not a tagged release, added the '.dev0'
# suffix to the package's version name.
#

define DIRTY_RELEASE
###############################################################################
# WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNIG WARNING
#
# This release build is not on a release tag. Its version will be modified to
# use the .dev suffix.
#
# Do not use in production. Do not distribute.
###############################################################################
endef
export DIRTY_RELEASE

RELEASE_TAG=$(shell \
	git describe --tags --match 'CLIENT_*' | \
	sed 's/\(CLIENT_[0-9]\+\.[0-9]\+\).*/\1/'\
)

# This target will check if the repo is clean (on a CLIENT_ tag with no extra
# cruft). If it is, then create the file 'clean' which signals setup.py to
# _not_ add the .dev0 suffix.
_check_release:
	@git diff --quiet $(RELEASE_TAG) -- . > /dev/null; \
	if [ $$? != 0 ]; \
	then \
		echo "$$DIRTY_RELEASE"; \
		rm -f clean; \
	else \
		touch clean; \
	fi

release: _check_release $(VENV_ROOT)
	$(PIP) install wheel
	$(SETUP_PY_ENV) $(PYTHON_EXE) setup.py bdist_wheel --universal

clean:
	rm -rf oauth_ssh.egg-info
	rm -rf $(VENV_NAME)
	find . -name '*pyc' -exec rm {} \;

.PHONY: develop test clean _check_release
