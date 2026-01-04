# OpenOMF Documentation

This directory contains the Sphinx documentation project for OpenOMF.

## Prerequisites

- Python 3.8+

## Setup

1. Create and activate a virtual environment:

```bash
cd doc
python3 -m venv venv
source venv/bin/activate
```

2. Install Python dependencies:

```bash
pip install -r requirements.txt
```

## Building Documentation

With the virtual environment activated:

```bash
make html
```

The generated documentation will be in `_build/html/`. Open `_build/html/index.html` in a browser to view it.

## Other Commands

```bash
make clean  # Remove all generated files
```

## Directory Structure

```
doc/
├── Makefile           # Build commands
├── README.md          # This file
├── requirements.txt   # Python dependencies
├── source/
│   ├── conf.py        # Sphinx configuration
│   └── index.rst      # Documentation root
└── _build/            # Generated HTML (git-ignored)
```

## Documenting C Code

Use Sphinx's built-in C domain to document code inline. Examples:

```rst
.. c:function:: int engine_init(engine_init_flags *init_flags)

   Initialize the game engine.

   :param init_flags: Initialization options
   :return: 0 on success, non-zero on failure

.. c:struct:: vec2i

   2D integer vector.

   .. c:member:: int x
   .. c:member:: int y
```

Reference documented items with ``:c:func:`engine_init``` or ``:c:struct:`vec2i```.
