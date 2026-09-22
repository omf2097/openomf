#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${OPENOMF_BUILD_DIR:-$ROOT_DIR/build/macos-app}"
DIST_DIR="${OPENOMF_DIST_DIR:-$ROOT_DIR/dist/macos}"
APP_NAME="${OPENOMF_APP_NAME:-OpenOMF}"
BUNDLE_ID="${OPENOMF_BUNDLE_ID:-org.openomf.OpenOMF}"
BUNDLE_VERSION="${OPENOMF_BUNDLE_VERSION:-0.0.0}"
BUNDLE_BUILD="${OPENOMF_BUNDLE_BUILD:-1}"
SIGN_IDENTITY="${OPENOMF_CODESIGN_IDENTITY:-}"
TEAM_ID="${OPENOMF_TEAM_ID:-}"
VCPKG_ROOT="${VCPKG_ROOT:-$ROOT_DIR/.vcpkg}"
DEFAULT_TRIPLET="x64-osx"
if [[ "$(uname -m)" == "arm64" ]]; then
    DEFAULT_TRIPLET="arm64-osx"
fi
VCPKG_TRIPLET="${VCPKG_TARGET_TRIPLET:-$DEFAULT_TRIPLET}"
APP_DIR="$DIST_DIR/$APP_NAME.app"
CONTENTS_DIR="$APP_DIR/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
RESOURCES_DIR="$CONTENTS_DIR/Resources"
APP_RESOURCE_ROOT="$RESOURCES_DIR/openomf"
ENTITLEMENTS="$ROOT_DIR/packaging/macos/openomf.entitlements"
ICON_SOURCE="$ROOT_DIR/packaging/macos/openomf.icns"
ICON_FILE="openomf.icns"

cmake_args=(
    -S "$ROOT_DIR"
    -B "$BUILD_DIR"
    -DCMAKE_BUILD_TYPE=Release
    -DUSE_TESTS=OFF
    -DUSE_TOOLS=OFF
    -DUSE_MINIUPNPC=OFF
    -DUSE_NATPMP=OFF
)

if [[ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]]; then
    cmake_args+=(
        -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
        -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET"
    )
fi

cmake "${cmake_args[@]}"
cmake --build "$BUILD_DIR" --config Release --target openomf

rm -rf "$APP_DIR"
mkdir -p "$MACOS_DIR" "$APP_RESOURCE_ROOT/resources" "$APP_RESOURCE_ROOT/shaders"

cp "$BUILD_DIR/openomf" "$MACOS_DIR/$APP_NAME"
cp "$ROOT_DIR/resources/openomf.bk" "$APP_RESOURCE_ROOT/resources/"
cp "$ROOT_DIR/resources/gamecontrollerdb/gamecontrollerdb.txt" "$APP_RESOURCE_ROOT/resources/"
cp -R "$ROOT_DIR/shaders/." "$APP_RESOURCE_ROOT/shaders/"

find "$BUILD_DIR/resources" -maxdepth 1 -type f \( -name "*.DAT2" -o -name "*.LNG" -o -name "*.LNG2" \) \
    -exec cp {} "$APP_RESOURCE_ROOT/resources/" \;

if [[ -d "$ROOT_DIR/data" ]]; then
    cp -R "$ROOT_DIR/data/." "$APP_RESOURCE_ROOT/resources/"
fi

if [[ ! -f "$ICON_SOURCE" ]]; then
    echo "Missing macOS icon source: $ICON_SOURCE" >&2
    exit 1
fi
cp "$ICON_SOURCE" "$RESOURCES_DIR/$ICON_FILE"

cat > "$CONTENTS_DIR/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDevelopmentRegion</key>
    <string>en</string>
    <key>CFBundleExecutable</key>
    <string>$APP_NAME</string>
    <key>CFBundleIconFile</key>
    <string>$ICON_FILE</string>
    <key>CFBundleIdentifier</key>
    <string>$BUNDLE_ID</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>$APP_NAME</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>$BUNDLE_VERSION</string>
    <key>CFBundleVersion</key>
    <string>$BUNDLE_BUILD</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.15</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
PLIST

search_dirs=("$BUILD_DIR")
if [[ -d "$VCPKG_ROOT/installed/$VCPKG_TRIPLET" ]]; then
    search_dirs+=("$VCPKG_ROOT/installed/$VCPKG_TRIPLET/lib")
fi

cmake \
    -DAPP="$APP_DIR" \
    -DSEARCH_DIRS="$(IFS=';'; echo "${search_dirs[*]}")" \
    -P "$ROOT_DIR/packaging/macos/fixup_bundle.cmake"

codesign_common_args=(--force)
if [[ -n "$SIGN_IDENTITY" ]]; then
    codesign_common_args+=(--timestamp --options runtime --sign "$SIGN_IDENTITY")
else
    codesign_common_args+=(--sign -)
fi

if [[ -d "$APP_DIR/Contents/Frameworks" ]]; then
    while IFS= read -r item; do
        codesign "${codesign_common_args[@]}" "$item"
    done < <(find "$APP_DIR/Contents/Frameworks" -type f -name "*.dylib")
fi

app_sign_args=("${codesign_common_args[@]}")
if [[ -n "$SIGN_IDENTITY" ]]; then
    app_sign_args+=(--entitlements "$ENTITLEMENTS")
fi
if [[ -n "$TEAM_ID" ]]; then
    app_sign_args+=(--team-id "$TEAM_ID")
fi
codesign "${app_sign_args[@]}" "$APP_DIR"
codesign --verify --deep --strict --verbose=2 "$APP_DIR"

echo "$APP_DIR"
