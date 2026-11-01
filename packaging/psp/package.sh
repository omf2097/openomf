#!/bin/sh
# Assemble the PSP distribution outside the cross-compiled CMake project.
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
BUILD_DIR=${1:?Usage: package.sh PSP_BUILD_DIR HOST_LANGUAGES_DIR}
LANGUAGES_DIR=${2:?Usage: package.sh PSP_BUILD_DIR HOST_LANGUAGES_DIR}
BUILD_DIR=$(CDPATH= cd -- "$BUILD_DIR" && pwd)
LANGUAGES_DIR=$(CDPATH= cd -- "$LANGUAGES_DIR" && pwd)
ASSET_ARCHIVE=${OMF_ASSET_ARCHIVE:-"$BUILD_DIR/openomf-assets.zip"}
GAME_DIR="$BUILD_DIR/PSP/GAME/OpenOMF"

if [ ! -s "$BUILD_DIR/EBOOT.PBP" ]; then
    echo "Missing PSP EBOOT: $BUILD_DIR/EBOOT.PBP" >&2
    exit 1
fi
if [ ! -f "$ASSET_ARCHIVE" ]; then
    echo "Downloading original OMF 2097 game data"
    curl -fL --retry 3 --retry-delay 2 \
        https://www.omf2097.com/pub/files/omf/openomf-assets.zip -o "$ASSET_ARCHIVE"
fi
unzip -tq "$ASSET_ARCHIVE" >/dev/null
mkdir -p "$GAME_DIR/resources"
unzip -jo -o "$ASSET_ARCHIVE" 'OMF2097/*' -d "$GAME_DIR/resources" >/dev/null
cp "$BUILD_DIR/EBOOT.PBP" "$GAME_DIR/EBOOT.PBP"
cp "$ROOT/resources/openomf.bk" "$ROOT/resources/gamecontrollerdb/gamecontrollerdb.txt" "$GAME_DIR/resources/"

# Language catalogs come from the native CMake build_languages target.
# This preserves the existing language counts and generates all translations.
if [ ! -s "$LANGUAGES_DIR/ENGLISH.DAT2" ]; then
    echo "Missing native-built language catalog: $LANGUAGES_DIR/ENGLISH.DAT2" >&2
    exit 1
fi
cp "$LANGUAGES_DIR"/* "$GAME_DIR/resources/"

for name in MAIN.BK INTRO.BK SOUNDS.DAT MENU.PSM ENGLISH.DAT ENGLISH.DAT2 openomf.bk; do
    test -s "$GAME_DIR/resources/$name" || { echo "Missing PSP resource: $name" >&2; exit 1; }
done
(cd "$BUILD_DIR" && cmake -E tar cf openomf-psp-sdl-null.zip --format=zip PSP)
printf 'Staged PSP game at %s\n' "$GAME_DIR"
