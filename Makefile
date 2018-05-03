all:
	$(error Usage: "make {develop|test|package} [PYTHON_VERSION=<x.y.z>]")

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
    CREATE_VIRTUAL_ENV=virtualenv
    VENV_ROOT=venv_$(PYTHON_VERSION)
    PYTHON_EXE=$(VENV_ROOT)/bin/python
    PIP=$(VENV_ROOT)/bin/pip
endif

GLOBUS_SSH=$(VENV_ROOT)/bin/globus-ssh
NOSETESTS=$(VENV_ROOT)/bin/nosetests
VENV_NAME=$(shell basename $(VENV_ROOT))

ifdef USING_PENV
$(PY_VER_ROOT):
	pyenv install --skip-existing $(PYTHON_VERSION)
endif

develop: $(GLOBUS_SSH)
	@echo "Use \". $(VENV_ROOT)/bin/activate\""

test: $(GLOBUS_SSH) $(NOSETESTS)
	$(NOSETESTS) -v -s

$(VENV_ROOT): $(PY_VER_ROOT)
	$(CREATE_VIRTUAL_ENV) $(VENV_NAME)
	$(PIP) install --upgrade pip
	$(PIP) install --upgrade setuptools

$(GLOBUS_SSH): $(VENV_ROOT)
	$(SETUP_PY_ENV) $(PYTHON_EXE) setup.py develop

$(NOSETESTS): $(VENV_ROOT)
	$(PIP) install nose


package: $(VENV_ROOT)
	$(PIP) install wheel
	$(SETUP_PY_ENV) $(PYTHON_EXE) setup.py bdist_wheel --universal

clean:
	rm -rf globus_ssh.egg-info
	rm -rf $(VENV_NAME)
	find . -name '*pyc' -exec rm {} \;

.PHONY: develop test
