#!/usr/bin/env bash

if [ -z "$1" ]; then
  echo "Usage: $0 <build-dir>" >&2
  exit 1
fi

export BUILD_DIR="$1"
OPENOMF_BIN=$(find "$BUILD_DIR" -name openomf -type f -executable -print -quit)
if [ -z "$OPENOMF_BIN" ]; then
  echo "Could not find openomf executable from $BUILD_DIR" >&2
  exit 1
fi
export OPENOMF_BIN="./${OPENOMF_BIN#$BUILD_DIR}"

exec poetry run pytest -vrP pytest
