# OpenOMF Documentation

This directory contains the Sphinx documentation project for OpenOMF.

## Prerequisites

- Python 3.8+
- Doxygen (for API documentation)
- Graphviz (for diagrams)

### Doxygen and Graphviz

- Arch Linux: `sudo pacman -S doxygen graphviz`
- Debian/Ubuntu: `sudo apt install doxygen graphviz`
- Fedora: `sudo dnf install doxygen graphviz`
- macOS: `brew install doxygen graphviz`

### Virtualenv

```bash
make venv
```

## Building Documentation

```bash
make html
```

The generated documentation will be in `_build/html/`. Open `_build/html/index.html` in a browser to view it.
