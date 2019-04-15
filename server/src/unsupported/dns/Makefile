VIRTUALENV=venv


$(VIRTUALENV):
	virtualenv $(VIRTUALENV)

develop: $(VIRTUALENV)
	$(VIRTUALENV)/bin/python setup.py develop

clean:
	rm -rf dist *.egg-info *pyc $(VIRTUALENV) build
