#!/usr/bin/env bash

if [ -z "$1" ]; then
    echo "Usage: $0 <build-dir>" >&2
    exit 1
fi

export BUILD_DIR="$1"
OPENOMF_BIN=$(find "$BUILD_DIR" "(" -name openomf -or -name openomf.exe ")" -type f -executable -print -quit)
if [ -z "$OPENOMF_BIN" ]; then
    echo "Could not find openomf executable from $BUILD_DIR" >&2
    exit 1
fi
export OPENOMF_BIN="./${OPENOMF_BIN#$BUILD_DIR}"

# Setup temp directory for outputs
temp_dir=$(mktemp -d)
trap 'rm -rf "$temp_dir"' EXIT

interrupt() {
    echo "Test run interrupted" >&2
    exit 1
}
trap interrupt INT

fail_count=0
fail_summary=""

echo "Running tests..."

RUNDIR=$(pwd)

cd $BUILD_DIR

export OPENOMF_RESOURCE_PATH="."
export LSAN_OPTIONS="suppressions=../lsan.supp"

output_file="$temp_dir/output_shouldfail.log"
if $OPENOMF_BIN --force-audio-backend=NULL --force-renderer=NULL --warp -P "$RUNDIR/rectests/SHOULDFAIL.REC" >"$output_file" 2>&1; then
    cat "$output_file"
    echo "CRITICAL ERROR: SHOULDFAIL.REC succeeded."
    exit 1
fi

cmd="\$OPENOMF_BIN --force-audio-backend=NULL --force-renderer=NULL --warp"

for filename in "$RUNDIR"/rectests/*.REC; do
    if [[ "$filename" == *SHOULDFAIL.REC ]]; then
        continue
    fi
    cmd="$cmd -P '$filename'"
    found_something=1
done

if [ -z "${found_something}" ]; then
    echo "Failed to find any rectests!"
    exit 1
fi

if RUNDIR="${RUNDIR}" OPENOMF_BIN="${OPENOMF_BIN}" sh -c "$cmd"; then
    echo "ALL PASSED."
else
    echo "RECTESTS FAILED, GOODBYE."
    exit 1
fi
