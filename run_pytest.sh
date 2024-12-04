#!/usr/bin/env bash

if [ -z "$2" ]; then
  echo "Usage: $0 <build-dir> <openomf-executable>" >&2
  exit 1
fi

export BUILD_DIR="$1"
export OPENOMF_BIN="$2"

exec poetry run pytest -vrP pytest
