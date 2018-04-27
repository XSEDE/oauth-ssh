VIRTUALENV=venv
NOSETESTS=$(VIRTUALENV)/bin/nosetests

$(VIRTUALENV):
	virtualenv $(VIRTUALENV)
	$(VIRTUALENV)/bin/pip install --upgrade pip
	$(VIRTUALENV)/bin/pip install --upgrade setuptools

develop: $(VIRTUALENV)
	$(VIRTUALENV)/bin/python setup.py develop

$(NOSETESTS): $(VIRTUALENV)
	$(VIRTUALENV)/bin/pip install nose

test: $(NOSETESTS)
	nosetests -v

package: $(VIRTUALENV)
	$(VIRTUALENV)/bin/pip install wheel
	$(VIRTUALENV)/bin/python setup.py bdist_wheel

clean:
	rm -rf globus_ssh.egg-info
	rm -rf venv
	find . -name '*pyc' -exec rm {} \;

.PHONY: clean develop test
