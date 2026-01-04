# Makefile for Sphinx documentation

SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = _build

.PHONY: help clean html

help:
	@echo "OpenOMF Documentation Build Targets:"
	@echo ""
	@echo "  make html   - Build HTML documentation"
	@echo "  make clean  - Remove all built documentation"
	@echo ""
	@echo "See README.md for setup instructions."

clean:
	rm -rf $(BUILDDIR)

html:
	$(SPHINXBUILD) -b html $(SOURCEDIR) $(BUILDDIR)/html $(SPHINXOPTS)
	@echo ""
	@echo "Build finished. Open $(BUILDDIR)/html/index.html in a browser."
