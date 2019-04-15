VIRTUALENV=venv

EXTRA_DIST=setup.py oauth_ssh_config

#
# buildrpm target
#
install:
	python setup.py install --root $(DESTDIR)/ --install-scripts /usr/sbin

$(VIRTUALENV):
	virtualenv $(VIRTUALENV)

develop: $(VIRTUALENV)
	$(VIRTUALENV)/bin/python setup.py develop

clean:
	rm -rf dist oauth_ssh_config.egg-info *pyc $(VIRTUALENV) build
