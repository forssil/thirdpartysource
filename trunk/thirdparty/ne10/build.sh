#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_DIR="$SCRIPT_DIR/../ne10source"
NDK_DIR="${ANDROID_NDK_HOME:-/Users/bytedance/work/A1T1/ndk/android-ndk-r28}"

TARGET="all"
ABI="arm64-v8a"
API_LEVEL="28"
OUTDIR=""

show_help() {
    cat <<EOF
Usage: $0 [all|clean] [OPTIONS]

Build Ne10 for Android with the local Android NDK.

Options:
  --abi <abi>       Android ABI: arm64-v8a or armeabi-v7a
  --api <level>     Android API level (default: 28)
  -o, --outdir      Output directory
  -h, --help        Show this help

Examples:
  $0
  $0 clean
  $0 all --abi armeabi-v7a
EOF
}

if [[ $# -gt 0 ]]; then
    case "$1" in
        all|clean)
            TARGET="$1"
            shift
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
    esac
fi

while [[ $# -gt 0 ]]; do
    case "$1" in
        --abi)
            ABI="$2"
            shift 2
            ;;
        --api)
            API_LEVEL="$2"
            shift 2
            ;;
        -o|--outdir)
            OUTDIR="$2"
            shift 2
            ;;
        -h|--help)
            show_help
            exit 0
            ;;
        *)
            echo "Error: unknown option: $1" >&2
            show_help
            exit 1
            ;;
    esac
done

case "$ABI" in
    arm64-v8a)
        NE10_ARCH="aarch64"
        DEFAULT_OUTDIR="$SCRIPT_DIR/android_arm64_v8a"
        ;;
    armeabi-v7a)
        NE10_ARCH="armv7"
        DEFAULT_OUTDIR="$SCRIPT_DIR/android_armeabi_v7a"
        ;;
    *)
        echo "Error: unsupported ABI: $ABI" >&2
        exit 1
        ;;
esac

OUTDIR="${OUTDIR:-$DEFAULT_OUTDIR}"
BUILD_DIR="$SCRIPT_DIR/build/$ABI"

if [[ "$TARGET" == "clean" ]]; then
    rm -rf "$BUILD_DIR" "$OUTDIR"
    echo "Cleaned: $BUILD_DIR"
    echo "Cleaned: $OUTDIR"
    exit 0
fi

if [[ ! -f "$SOURCE_DIR/CMakeLists.txt" ]]; then
    echo "Error: Ne10 source not found at $SOURCE_DIR" >&2
    echo "Run: git submodule update --init trunk/thirdparty/ne10source" >&2
    exit 1
fi

TOOLCHAIN_FILE="$NDK_DIR/build/cmake/android.toolchain.cmake"
if [[ ! -f "$TOOLCHAIN_FILE" ]]; then
    echo "Error: Android NDK toolchain not found at $TOOLCHAIN_FILE" >&2
    exit 1
fi

case "$(uname -s)" in
    Darwin)
        NDK_HOST_TAG="darwin-x86_64"
        ;;
    Linux)
        NDK_HOST_TAG="linux-x86_64"
        ;;
    *)
        echo "Error: unsupported build host: $(uname -s)" >&2
        exit 1
        ;;
esac

NDK_SYSROOT="$NDK_DIR/toolchains/llvm/prebuilt/$NDK_HOST_TAG/sysroot"

if command -v nproc >/dev/null 2>&1; then
    CORES="$(nproc)"
elif command -v sysctl >/dev/null 2>&1; then
    CORES="$(sysctl -n hw.ncpu)"
else
    CORES=4
fi

echo "Ne10 source : $SOURCE_DIR"
echo "Android NDK : $NDK_DIR"
echo "ABI         : $ABI"
echo "API level   : $API_LEVEL"
echo "Build type  : Release"
echo "Output      : $OUTDIR"

cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_BUILD_TYPE=Release \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="android-$API_LEVEL" \
    -DANDROID_API_LEVEL="$API_LEVEL" \
    -DNE10_ANDROID_TARGET_ARCH="$NE10_ARCH" \
    -DNDK_SYSROOT_PATH="$NDK_SYSROOT" \
    -DNDK_ISYSROOT_PATH="$NDK_SYSROOT" \
    -DNE10_BUILD_STATIC=ON \
    -DNE10_BUILD_SHARED=OFF \
    -DNE10_BUILD_EXAMPLES=OFF \
    -DNE10_BUILD_UNIT_TEST=OFF \
    -DBUILD_DEBUG=OFF

cmake --build "$BUILD_DIR" --target NE10 -j"$CORES"

LIBRARY="$BUILD_DIR/modules/libNE10.a"
if [[ ! -f "$LIBRARY" ]]; then
    echo "Error: build output not found: $LIBRARY" >&2
    exit 1
fi

mkdir -p "$OUTDIR/lib" "$OUTDIR/include"
cp "$LIBRARY" "$OUTDIR/lib/"
cmake -E copy_directory "$SOURCE_DIR/inc" "$OUTDIR/include"

echo "Built: $OUTDIR/lib/libNE10.a"
echo "Headers: $OUTDIR/include"
