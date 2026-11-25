#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TRUNK_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
WORKSPACE_ROOT="$(cd "$TRUNK_DIR/../.." && pwd)"
cd "$SCRIPT_DIR"

show_help()
{
    cat <<EOF
Usage: $0 [COMMAND] [OPTIONS]

Commands:
  all       Build shared and static libraries (default)
  share     Build libAPF.so
  static    Build libAPF.a
  test      Run ARM64 source, parity, and benchmark tests
  clean     Remove artifacts for the selected platform

Toolchain options:
  -c, --cross-compile <prefix>  Cross-compiler prefix
  --cc <path>                   C compiler override
  --cxx <path>                  C++ compiler override
  --ld <path>                   Linker override
  --ar <path>                   Archiver override

Android options:
  --ndk <aarch64|arm>           Build with Android NDK
  --ndk-path <path>             NDK root (default: ANDROID_NDK_HOME or workspace NDK)
  --api-level <level>           Android API level (default: 28)

SigmaStar options:
  --sigmastar                   Use the native SigmaStar cross-toolchain
  --sigmastar-prefix <prefix>   SigmaStar compiler prefix
  --sigmastar-docker            Build SigmaStar libraries through Docker

Output options:
  -o, --outdir <path>           Library output directory
  --objdir <path>               Object output directory
  -j, --jobs <count>            Parallel build jobs

Examples:
  $0 all --ndk aarch64
  $0 static --ndk aarch64 --ndk-path /opt/android-ndk-r28
  $0 test
  $0 clean --ndk aarch64
  $0 all --sigmastar-docker

Detailed Android ARM64 API 21 example:
  $0 all \\
    --ndk aarch64 \\
    --api-level 21 \\
    --ndk-path "$WORKSPACE_ROOT/ndk/android-ndk-r28" \\
    --objdir "$WORKSPACE_ROOT/output/obj/android_arm64_v8a_api21" \\
    --outdir "$WORKSPACE_ROOT/output/android_arm64_v8a" \\
    --jobs 8

Outputs:
  $WORKSPACE_ROOT/output/android_arm64_v8a/libAPF.so
  $WORKSPACE_ROOT/output/android_arm64_v8a/libAPF.a
  $WORKSPACE_ROOT/output/android_arm64_v8a/libc++_shared.so
EOF
}

fail()
{
    echo "Error: $*" >&2
    exit 1
}

require_value()
{
    [ "$#" -ge 2 ] && [ -n "$2" ] || fail "Option $1 requires a value"
}

detect_jobs()
{
    if command -v nproc >/dev/null 2>&1; then
        nproc
    elif command -v sysctl >/dev/null 2>&1; then
        sysctl -n hw.ncpu
    else
        echo 4
    fi
}

find_ndk_bin()
{
    local host_tag
    local candidate

    case "$(uname -s)-$(uname -m)" in
        Darwin-arm64) host_tag="darwin-arm64" ;;
        Darwin-*) host_tag="darwin-x86_64" ;;
        Linux-*) host_tag="linux-x86_64" ;;
        *) fail "Unsupported NDK host: $(uname -s)-$(uname -m)" ;;
    esac

    candidate="$NDK_PATH/toolchains/llvm/prebuilt/$host_tag/bin"
    if [ ! -d "$candidate" ] && [ "$host_tag" = "darwin-arm64" ]; then
        candidate="$NDK_PATH/toolchains/llvm/prebuilt/darwin-x86_64/bin"
    fi
    [ -d "$candidate" ] || fail "NDK toolchain directory not found under $NDK_PATH"
    printf '%s\n' "$candidate"
}

check_executable()
{
    if [[ "$1" == */* ]]; then
        [ -x "$1" ] || fail "Required executable not found: $1"
    else
        command -v "$1" >/dev/null 2>&1 ||
            fail "Required executable not found in PATH: $1"
    fi
}

COMMAND="all"
case "${1:-}" in
    all|share|static|test|clean)
        COMMAND="$1"
        shift
        ;;
    -h|--help)
        show_help
        exit 0
        ;;
esac

PLATFORM="generic"
NDK_ARCH=""
NDK_PATH="${ANDROID_NDK_HOME:-$WORKSPACE_ROOT/ndk/android-ndk-r28}"
API_LEVEL=28
SIGMASTAR_PREFIX="${SIGMASTAR_TOOLCHAIN_PREFIX:-}"
CROSS_PREFIX="${CROSS_COMPILE:-}"
CC_OVERRIDE=""
CXX_OVERRIDE=""
LD_OVERRIDE=""
AR_OVERRIDE=""
OUTDIR_VALUE="${OUTDIR:-}"
OBJDIR_VALUE="${OBJ_PATH:-}"
JOBS="$(detect_jobs)"

while [ "$#" -gt 0 ]; do
    case "$1" in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--cross-compile)
            require_value "$@"
            CROSS_PREFIX="$2"
            shift 2
            ;;
        -o|--outdir)
            require_value "$@"
            OUTDIR_VALUE="$2"
            shift 2
            ;;
        --objdir)
            require_value "$@"
            OBJDIR_VALUE="$2"
            shift 2
            ;;
        --cc)
            require_value "$@"
            CC_OVERRIDE="$2"
            shift 2
            ;;
        --cxx)
            require_value "$@"
            CXX_OVERRIDE="$2"
            shift 2
            ;;
        --ld)
            require_value "$@"
            LD_OVERRIDE="$2"
            shift 2
            ;;
        --ar)
            require_value "$@"
            AR_OVERRIDE="$2"
            shift 2
            ;;
        --ndk)
            require_value "$@"
            [ "$PLATFORM" = "generic" ] || fail "Only one platform may be selected"
            PLATFORM="android"
            NDK_ARCH="$2"
            shift 2
            ;;
        --ndk-path)
            require_value "$@"
            NDK_PATH="$2"
            shift 2
            ;;
        --api-level)
            require_value "$@"
            API_LEVEL="$2"
            shift 2
            ;;
        --sigmastar)
            [ "$PLATFORM" = "generic" ] || fail "Only one platform may be selected"
            PLATFORM="sigmastar"
            shift
            ;;
        --sigmastar-prefix)
            require_value "$@"
            SIGMASTAR_PREFIX="$2"
            shift 2
            ;;
        --sigmastar-docker)
            [ "$PLATFORM" = "generic" ] || fail "Only one platform may be selected"
            PLATFORM="sigmastar-docker"
            shift
            ;;
        -j|--jobs)
            require_value "$@"
            JOBS="$2"
            shift 2
            ;;
        *)
            fail "Unknown option: $1"
            ;;
    esac
done

case "$API_LEVEL" in
    *[!0-9]*|"") fail "API level must be a positive integer" ;;
esac
[ "$API_LEVEL" -gt 0 ] || fail "API level must be a positive integer"
case "$JOBS" in
    *[!0-9]*|"") fail "Job count must be a positive integer" ;;
esac
[ "$JOBS" -gt 0 ] || fail "Job count must be a positive integer"

TARGET_ARCH_VALUE="generic"
PLATFORM_ID="generic"
OBJ_PLATFORM_ID=""
NDK_RUNTIME_TRIPLE=""
NDK_READELF=""
NE10_STATIC_LIB=""
DOCKER_WRAPPER="${SIGMASTAR_DOCKER_WRAPPER:-$WORKSPACE_ROOT/docker/sigmastar/sigmastar-docker}"

case "$PLATFORM" in
    android)
        [ -d "$NDK_PATH" ] || fail "NDK path not found: $NDK_PATH"
        NDK_BIN="$(find_ndk_bin)"
        case "$NDK_ARCH" in
            aarch64)
                PLATFORM_ID="android_arm64_v8a"
                TARGET_ARCH_VALUE="arm64"
                COMPILER_TRIPLE="aarch64-linux-android"
                NDK_RUNTIME_TRIPLE="aarch64-linux-android"
                ;;
            arm)
                PLATFORM_ID="android_armeabi_v7a"
                TARGET_ARCH_VALUE="arm"
                COMPILER_TRIPLE="armv7a-linux-androideabi"
                NDK_RUNTIME_TRIPLE="arm-linux-androideabi"
                ;;
            *)
                fail "Unsupported NDK architecture '$NDK_ARCH'; use aarch64 or arm"
                ;;
        esac
        OBJ_PLATFORM_ID="${PLATFORM_ID}_api${API_LEVEL}"
        CC_VALUE="$NDK_BIN/${COMPILER_TRIPLE}${API_LEVEL}-clang"
        CXX_VALUE="$NDK_BIN/${COMPILER_TRIPLE}${API_LEVEL}-clang++"
        LD_VALUE="$NDK_BIN/ld.lld"
        AR_VALUE="$NDK_BIN/llvm-ar"
        NDK_READELF="$NDK_BIN/llvm-readelf"
        NE10_ROOT="$TRUNK_DIR/thirdparty/ne10/$PLATFORM_ID"
        NE10_STATIC_LIB="$NE10_ROOT/lib/libNE10.a"
        [ -f "$NE10_ROOT/include/NE10.h" ] ||
            fail "NE10 header not found: $NE10_ROOT/include/NE10.h"
        [ -f "$NE10_STATIC_LIB" ] ||
            fail "NE10 static library not found: $NE10_STATIC_LIB"
        export NE10_INCLUDE_DIR="$NE10_ROOT/include"
        export NE10_LIBS="$NE10_STATIC_LIB -lm"
        export DEFINES="${DEFINES:--fPIC -fpermissive -Wl,-soname,libAPF.so -DARM_NEON}"
        ;;
    sigmastar)
        PLATFORM_ID="sigmastar"
        if [ -z "$SIGMASTAR_PREFIX" ]; then
            SIGMASTAR_PREFIX="/home/gaohua.karl/code/A1/arm-sigmastar-linux-uclibcgnueabihf-9.1.0/bin/arm-sigmastar-linux-uclibcgnueabihf-9.1.0-"
        fi
        CC_VALUE="${SIGMASTAR_PREFIX}gcc"
        CXX_VALUE="${SIGMASTAR_PREFIX}g++"
        LD_VALUE="${SIGMASTAR_PREFIX}ld"
        AR_VALUE="${SIGMASTAR_PREFIX}ar"
        NE10_ROOT="$TRUNK_DIR/thirdparty/ne10/sigmastar"
        NE10_STATIC_LIB="$NE10_ROOT/lib/libNE10.a"
        [ -f "$NE10_ROOT/include/NE10.h" ] ||
            fail "NE10 header not found: $NE10_ROOT/include/NE10.h"
        [ -f "$NE10_STATIC_LIB" ] ||
            fail "NE10 static library not found: $NE10_STATIC_LIB"
        export NE10_INCLUDE_DIR="$NE10_ROOT/include"
        export NE10_LIBS="$NE10_STATIC_LIB -lm"
        export DEFINES="${DEFINES:--fPIC -fpermissive -Wl,-rpath=. -Wl,-soname,libAPF.so -DARM_NEON}"
        ;;
    sigmastar-docker)
        PLATFORM_ID="sigmastar"
        [ -x "$DOCKER_WRAPPER" ] || fail "Docker wrapper not found: $DOCKER_WRAPPER"
        ;;
    generic)
        CC_VALUE="${CC:-${CROSS_PREFIX}gcc}"
        CXX_VALUE="${CXX:-${CROSS_PREFIX}g++}"
        LD_VALUE="${LD:-${CROSS_PREFIX}ld}"
        AR_VALUE="${AR:-${CROSS_PREFIX}ar}"
        ;;
esac

if [ "$COMMAND" = "test" ]; then
    [ "$PLATFORM" = "generic" ] ||
        fail "The test command runs native ARM64 binaries and cannot be cross-compiled"
    case "$(uname -m)" in
        arm64|aarch64) ;;
        *) fail "The test command requires an ARM64 host" ;;
    esac
    PLATFORM_ID="native_arm64_test"
    TARGET_ARCH_VALUE="arm64"
    CC_VALUE="${CC:-clang}"
    CXX_VALUE="${CXX:-clang++}"
    LD_VALUE="${LD:-ld}"
    AR_VALUE="${AR:-ar}"
    export DEFINES="${DEFINES:--fPIC -DHAVE_LRINT=1 -DHAVE_LRINTF=1}"
fi

[ -n "$CC_OVERRIDE" ] && CC_VALUE="$CC_OVERRIDE"
[ -n "$CXX_OVERRIDE" ] && CXX_VALUE="$CXX_OVERRIDE"
[ -n "$LD_OVERRIDE" ] && LD_VALUE="$LD_OVERRIDE"
[ -n "$AR_OVERRIDE" ] && AR_VALUE="$AR_OVERRIDE"

if [ "$PLATFORM" != "sigmastar-docker" ]; then
    check_executable "$CC_VALUE"
    check_executable "$CXX_VALUE"
    check_executable "$AR_VALUE"
fi

if [ -z "$OUTDIR_VALUE" ]; then
    if [ "$PLATFORM" = "generic" ] && [ "$COMMAND" != "test" ]; then
        OUTDIR_VALUE="$TRUNK_DIR/libs"
    else
        OUTDIR_VALUE="$WORKSPACE_ROOT/output/$PLATFORM_ID"
    fi
fi
if [ -z "$OBJDIR_VALUE" ]; then
    OBJDIR_VALUE="$WORKSPACE_ROOT/output/obj/${OBJ_PLATFORM_ID:-$PLATFORM_ID}"
fi

export CC="$CC_VALUE"
export CXX="$CXX_VALUE"
export LD="$LD_VALUE"
export AR="$AR_VALUE"
export CROSS_COMPILE="$CROSS_PREFIX"
export TARGET_ARCH="$TARGET_ARCH_VALUE"
export OUTDIR="$OUTDIR_VALUE"
export OBJ_PATH="$OBJDIR_VALUE"

echo "---------------------------------------------------"
echo "Command:       $COMMAND"
echo "Platform:      $PLATFORM_ID"
echo "CC:            $CC"
echo "CXX:           $CXX"
echo "AR:            $AR"
echo "TARGET_ARCH:   $TARGET_ARCH"
echo "OBJ_PATH:      $OBJ_PATH"
echo "OUTDIR:        $OUTDIR"
[ -n "$NE10_STATIC_LIB" ] && echo "NE10 archive:  $NE10_STATIC_LIB"
echo "Jobs:          $JOBS"
echo "---------------------------------------------------"

mkdir -p "$OBJ_PATH" "$OUTDIR"

if [ "$PLATFORM" = "sigmastar-docker" ]; then
    DOCKER_OUTDIR="$TRUNK_DIR/libs/sigmastar"
    "$DOCKER_WRAPPER" make "$TRUNK_DIR" \
        -C sorce/AudioProcessingAlgorithm clean \
        OUTDIR=/work/libs/sigmastar \
        OBJ_PATH=/work/objs/sigmastar
    if [ "$COMMAND" != "clean" ]; then
        "$DOCKER_WRAPPER" make "$TRUNK_DIR" \
            -C sorce/AudioProcessingAlgorithm "-j$JOBS" "$COMMAND" \
            OUTDIR=/work/libs/sigmastar \
            OBJ_PATH=/work/objs/sigmastar \
            "NE10_LIBS=/work/thirdparty/ne10/sigmastar/lib/libNE10.a -lm"
        mkdir -p "$OUTDIR"
        [ ! -f "$DOCKER_OUTDIR/libAPF.so" ] ||
            cp "$DOCKER_OUTDIR/libAPF.so" "$OUTDIR/"
        [ ! -f "$DOCKER_OUTDIR/libAPF.a" ] ||
            cp "$DOCKER_OUTDIR/libAPF.a" "$OUTDIR/"
    fi
else
    MAKE_TARGET="$COMMAND"
    [ "$COMMAND" != "test" ] || MAKE_TARGET="arm64-function-parity-test"
    make -j"$JOBS" "$MAKE_TARGET"
fi

if [ "$COMMAND" != "clean" ] && [ "$COMMAND" != "test" ]; then
    case "$COMMAND" in
        all|share)
            [ -f "$OUTDIR/libAPF.so" ] || fail "Missing output: $OUTDIR/libAPF.so"
            file "$OUTDIR/libAPF.so"
            ;;
    esac
    case "$COMMAND" in
        all|static)
            [ -f "$OUTDIR/libAPF.a" ] || fail "Missing output: $OUTDIR/libAPF.a"
            file "$OUTDIR/libAPF.a"
            ;;
    esac
fi

if [ "$PLATFORM" = "android" ] && [ "$COMMAND" != "clean" ]; then
    if [ -f "$OUTDIR/libAPF.so" ]; then
        DYNAMIC_INFO="$("$NDK_READELF" -d "$OUTDIR/libAPF.so")"
        if printf '%s\n' "$DYNAMIC_INFO" | grep -q 'libNE10\.so'; then
            fail "libAPF.so has an unexpected dynamic dependency on libNE10.so"
        fi
        echo "Verified: libAPF.so links NE10 statically"
    fi
    if [ -f "$OUTDIR/libAPF.so" ]; then
        NDK_RUNTIME="$NDK_PATH/toolchains/llvm/prebuilt/$(basename "$(dirname "$(dirname "$CC")")")/sysroot/usr/lib/$NDK_RUNTIME_TRIPLE/libc++_shared.so"
        if [ -f "$NDK_RUNTIME" ]; then
            cp "$NDK_RUNTIME" "$OUTDIR/"
            echo "Copied: $OUTDIR/libc++_shared.so"
        else
            echo "Warning: libc++_shared.so not found at $NDK_RUNTIME" >&2
        fi
    fi
fi

echo "Done."
