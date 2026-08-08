#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TRUNK_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"

SIGMA_PREFIX="${SIGMASTAR_TOOLCHAIN_PREFIX:-/home/gaohua.karl/code/A1/arm-sigmastar-linux-uclibcgnueabihf-9.1.0/bin/arm-sigmastar-linux-uclibcgnueabihf-9.1.0-}"
APF_ROOT="${APF_ROOT:-$TRUNK_DIR/../../output/sigmastar}"
OUTDIR="${OUTDIR:-$SCRIPT_DIR/output/sigmastar}"
TARGET="$OUTDIR/linux_test"
DEFINES="${DEFINES:--fPIC -DARM_NEON}"

for tool in gcc g++ ar; do
    if [[ ! -x "${SIGMA_PREFIX}${tool}" ]]; then
        echo "Error: SigmaStar toolchain executable not found: ${SIGMA_PREFIX}${tool}" >&2
        exit 1
    fi
done

if [[ ! -f "$APF_ROOT/libAPF.so" ]]; then
    echo "Error: libAPF.so not found at $APF_ROOT" >&2
    echo "Build it first with: sorce/AudioProcessingAlgorithm/build.sh all --sigmastar" >&2
    exit 1
fi

mkdir -p "$OUTDIR"

echo "=> Building SigmaStar unit-test program"
echo "   CXX:    ${SIGMA_PREFIX}g++"
echo "   APF:    $APF_ROOT/libAPF.so"
echo "   Output: $TARGET"

"${SIGMA_PREFIX}g++" \
    -Wall -O3 $DEFINES \
    -I"$TRUNK_DIR/sorce/AudioProcessingAlgorithm/audio_processing/include" \
    -I"$TRUNK_DIR/sorce/AudioProcessingAlgorithm" \
    -I"$TRUNK_DIR/thirdparty/ne10/sigmastar/include" \
    "$SCRIPT_DIR/linux_test.cpp" \
    -L"$APF_ROOT" -Wl,-rpath,'$ORIGIN' -lAPF \
    -ldl -lpthread -lm \
    -o "$TARGET"

cp "$APF_ROOT/libAPF.so" "$OUTDIR/"
echo "Built: $TARGET"
echo "Copied: $OUTDIR/libAPF.so"
