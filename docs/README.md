# OpenOMF Documentation

This directory contains the Sphinx documentation project for OpenOMF. This project
is automatically built by readthedocs.org!

## Prerequisites

Graphviz is required for rendering architecture diagrams:

```bash
# Debian/Ubuntu
sudo apt install graphviz

# Fedora
sudo dnf install graphviz

# Arch Linux
sudo pacman -S graphviz

# macOS
brew install graphviz
```

## Setup

```bash
make venv
```

## Building Documentation

```bash
make html
```
