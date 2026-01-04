# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import subprocess

# -- Project information -----------------------------------------------------

project = 'OpenOMF'
copyright = '2026, OpenOMF Project Team'
author = 'OpenOMF Project Team'

# -- General configuration ---------------------------------------------------

extensions = [
    'sphinx.ext.graphviz',
    'breathe',
]

# Graphviz settings
graphviz_output_format = 'svg'

# Breathe settings - bridge between Doxygen and Sphinx
breathe_projects = {'openomf': os.path.abspath('../_build/doxygen/xml')}
breathe_default_project = 'openomf'
breathe_domain_by_extension = {'h': 'c', 'c': 'c'}

templates_path = ['_templates']
exclude_patterns = []

# Suppress harmless duplicate declaration warnings from Breathe
suppress_warnings = ['duplicate_declaration.c', 'duplicate_declaration.cpp']

# -- Options for HTML output -------------------------------------------------

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

# Set primary domain to C for code documentation
primary_domain = 'c'

# -- Run Doxygen before Sphinx -----------------------------------------------

def run_doxygen(app):
    """Run doxygen before sphinx build."""
    doxygen_dir = os.path.abspath(os.path.join(app.srcdir, '..'))
    subprocess.run(['doxygen', 'Doxyfile'], cwd=doxygen_dir, check=True)

def setup(app):
    app.connect('builder-inited', run_doxygen)
