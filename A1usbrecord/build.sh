#!/bin/bash

# 发生错误时立即退出
set -e

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 默认参数
NDK_DIR="/Users/bytedance/work/A1T1/ndk/android-ndk-r28"
ABI="arm64-v8a"
MIN_SDK="28"
BUILD_DIR="build"
OUTDIR="/Users/bytedance/work/A1T1/output/android_arm64_v8a"

function show_help() {
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  all       (Default) 编译 Android 版本"
    echo "  clean     清理编译产物 (build 目录)"
    echo ""
    echo "Options:"
    echo "  --abi <abi>     指定 Android ABI (例如: arm64-v8a, armeabi-v7a). 默认: $ABI"
    echo "  -o, --outdir    指定输出目录 (编译产物 av_virtual 将复制到此处). 默认: $OUTDIR"
    echo ""
    echo "Examples:"
    echo "  $0                      # 使用默认配置编译 arm64-v8a"
    echo "  $0 clean                # 清理"
    echo "  $0 all --abi armeabi-v7a -o /tmp/output  # 编译 32 位版本并指定输出"
}

TARGET="all"

# 解析第一个参数（如果是 clean，直接处理）
if [ "$1" == "clean" ]; then
    TARGET="clean"
    shift
elif [ "$1" == "all" ]; then
    TARGET="all"
    shift
elif [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
    show_help
    exit 0
fi

# 解析其余参数
while [[ $# -gt 0 ]]; do
    case $1 in
        --abi)
            ABI="$2"
            if [ "$ABI" == "armeabi-v7a" ]; then
                OUTDIR="/Users/bytedance/work/A1T1/output/android_armeabi_v7a"
            elif [ "$ABI" == "arm64-v8a" ]; then
                OUTDIR="/Users/bytedance/work/A1T1/output/android_arm64_v8a"
            fi
            shift 2
            ;;
        -o|--outdir)
            OUTDIR="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

if [ "$TARGET" == "clean" ]; then
    echo "=> Cleaning build directory: $BUILD_DIR"
    rm -rf "$BUILD_DIR"
    exit 0
fi

echo "=> Target Platform: Android Only"
echo "=> NDK Directory: $NDK_DIR"
echo "=> Target ABI: $ABI"
echo "=> Min SDK: $MIN_SDK"
echo "=> Output Directory: $OUTDIR"

# 获取 CPU 核心数以加速编译 (-j 参数)
# 兼容 macOS 和 Linux
if command -v nproc > /dev/null 2>&1; then
    CORES=$(nproc)
elif command -v sysctl > /dev/null 2>&1; then
    CORES=$(sysctl -n hw.ncpu)
else
    CORES=4
fi

echo "---------------------------------------------------"
echo "=> Running CMake configuration..."

mkdir -p "$BUILD_DIR"

# 使用 Android NDK 提供的 toolchain 进行交叉编译
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_TOOLCHAIN_FILE="$NDK_DIR/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="$ABI" \
    -DANDROID_PLATFORM="android-$MIN_SDK" \
    -DCMAKE_BUILD_TYPE=Release

echo "=> Running CMake build (-j${CORES})..."
cmake --build "$BUILD_DIR" -j"$CORES"

echo "---------------------------------------------------"
if [ -f "$BUILD_DIR/av_virtual" ]; then
    mkdir -p "$OUTDIR"
    cp "$BUILD_DIR/av_virtual" "$OUTDIR/"
    echo "=> Copied av_virtual to $OUTDIR"

    # 使用 NDK 自带的 llvm-strip 剔除符号表，减小体积
    STRIP="$NDK_DIR/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-strip"
    if [ -f "$STRIP" ]; then
        "$STRIP" "$OUTDIR/av_virtual"
        echo "=> Stripped $OUTDIR/av_virtual"
    fi
else
    echo "Error: av_virtual not found in $BUILD_DIR"
    exit 1
fi

echo "Done!"
