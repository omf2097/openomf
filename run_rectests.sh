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

# Define your tests here (description:filename)
tests=(
    "Overhead throw should throw opponent to the right:OHT.REC"
    "Electra should be able to kick while doing inputs for rolling thunder:65K6P.REC"
    "Jaguar uses same animation for K and 6P but Shadow does not:6P.REC"
    "Novas 1P can hit more than once with the Q tag and use AI for air launch:NOVA-1P.REC"
    "Jaguar standing throw should not result in overlap with opponent in corner:JAG-CORNER-THROW.REC"
    "Jaguar doing a standing throw as the winning move should not cause a slide:JAG-NO-SLIDE-WIN-THROW.REC"
    "Flail charging punch should cancel when blocked:FLAIL_454P.REC"
)

fail_count=0

echo "Running tests..."

RUNDIR=$(pwd)

cd $BUILD_DIR

export ASAN_OPTIONS=detect_leaks=0

for test in "${tests[@]}"; do
    IFS=':' read -r desc filename <<< "$test"
    # Trim whitespace from description and filename
    desc=$(echo "$desc" | xargs)
    filename=$(echo "$filename" | xargs)

    echo -n "${desc} :"
    if $OPENOMF_BIN --force-audio-backend=NULL --force-renderer=NULL --speed=10 -P "$RUNDIR/rectests/${filename}" >/dev/null 2>&1; then
        echo " PASS"
    else
        echo " FAILED"
        ((fail_count++))
    fi
done

# Exit with non-zero status if any test failed
exit $fail_count
