# Makefile for Sphinx documentation

SPHINXOPTS    ?=
VENV          = venv
SPHINXBUILD   = $(VENV)/bin/sphinx-build
PYTHON        = $(VENV)/bin/python
SOURCEDIR     = source
BUILDDIR      = _build

.PHONY: clean html venv activate

venv:
	python3 -m venv $(VENV)
	$(VENV)/bin/pip install --upgrade pip
	$(VENV)/bin/pip install -r requirements.txt

clean:
	rm -rf $(BUILDDIR)

html:
	$(SPHINXBUILD) -b html $(SOURCEDIR) $(BUILDDIR)/html $(SPHINXOPTS)
