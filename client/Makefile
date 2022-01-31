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
NOSETESTS=$(VENV_ROOT)/bin/nosetests
VENV_NAME=$(shell basename $(VENV_ROOT))

ifdef USING_PENV
$(PY_VER_ROOT):
	pyenv install --skip-existing $(PYTHON_VERSION)
endif

develop: $(OAUTH_SSH)
	@echo "Use \". $(VENV_ROOT)/bin/activate\""

test: $(OAUTH_SSH) $(NOSETESTS)
	$(NOSETESTS) -v -s

$(VENV_ROOT): $(PY_VER_ROOT)
	$(CREATE_VIRTUAL_ENV) $(VENV_NAME)
	$(PIP) install --upgrade pip
	$(PIP) install --upgrade setuptools

$(OAUTH_SSH): $(VENV_ROOT)
	$(SETUP_PY_ENV) $(PYTHON_EXE) setup.py develop

$(NOSETESTS): $(VENV_ROOT)
	$(PIP) install nose

#
# _check_release warns if we are building a release but either we
# are not on a tag or the tree is dirty.
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

_check_release:
	@git diff --quiet $(RELEASE_TAG) -- . > /dev/null; \
	if [ $$? != 0 ]; \
	then \
		echo "$$DIRTY_RELEASE"; \
		sed -i '0,/^\(version\s*=\s*[0-9]\+\.[0-9]\+\).*/s//\1.dev/' setup.cfg; \
	fi

release: _check_release $(VENV_ROOT)
	$(PIP) install build
	# --no-isolation avoids a conflict with virtualenv's bad site.py
	$(PYTHON_EXE) -m build --no-isolation

release: _check_release $(VENV_ROOT)
	$(PIP) install wheel
	$(SETUP_PY_ENV) $(PYTHON_EXE) setup.py bdist_wheel --universal

clean:
	rm -rf oauth_ssh.egg-info
	rm -rf $(VENV_NAME)
	find . -name '*pyc' -exec rm {} \;

.PHONY: develop test clean _check_release
