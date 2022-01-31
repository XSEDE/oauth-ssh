EXTRA_DIST=setup.cfg setup.py oauth_ssh_config

#
# buildrpm target
#
install:
	python3 setup.py install --root $(DESTDIR)/ --install-scripts /usr/sbin

venv/bin/python3:
	python3 -m venv venv

develop: venv/bin/python3
	venv/bin/python3 setup.py develop

clean:
	rm -rf dist oauth_ssh_config.egg-info *pyc $(VIRTUALENV) build
