#!/bin/bash
# Android 测试程序编译脚本
# 用于编译 linux_test.cpp 为 Android 可执行文件，支持 adb 调试

set -e

# 默认配置
NDK_PATH="/Users/bytedance/work/A1T1/ndk/android-ndk-r28"
API_LEVEL=28
ARCH="aarch64"
OUTDIR="./android_build"
BUILD_LIB="yes"

# 解析参数
while [[ $# -gt 0 ]]; do
    case "$1" in
        --arch)
            ARCH="$2"
            shift 2
            ;;
        --ndk)
            NDK_PATH="$2"
            shift 2
            ;;
        --api)
            API_LEVEL="$2"
            shift 2
            ;;
        --outdir)
            OUTDIR="$2"
            shift 2
            ;;
        --no-lib)
            BUILD_LIB="no"
            shift 1
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  --arch <arch>      Target architecture: aarch64 or arm (default: aarch64)"
            echo "  --ndk <path>       Path to NDK (default: $NDK_PATH)"
            echo "  --api <level>      Android API level (default: $API_LEVEL)"
            echo "  --outdir <path>    Output directory (default: $OUTDIR)"
            echo "  --no-lib           Skip building libAPF.so (assume already built)"
            echo "  --help             Show this help"
            exit 0
            ;;
        *)
            shift
            ;;
    esac
done

# 设置 NDK 工具链路径
NDK_BIN="$NDK_PATH/toolchains/llvm/prebuilt/darwin-x86_64/bin"

# 根据架构设置编译器
if [ "$ARCH" == "aarch64" ]; then
    CC="$NDK_BIN/aarch64-linux-android$API_LEVEL-clang"
    CXX="$NDK_BIN/aarch64-linux-android$API_LEVEL-clang++"
    TARGET="linux_test_arm64"
    LIB_OUTDIR="/Users/bytedance/work/A1T1/output/android_arm64_v8a"
    echo "=> Using NDK environment for Android aarch64 (API $API_LEVEL)"
elif [ "$ARCH" == "arm" ]; then
    CC="$NDK_BIN/armv7a-linux-androideabi$API_LEVEL-clang"
    CXX="$NDK_BIN/armv7a-linux-androideabi$API_LEVEL-clang++"
    TARGET="linux_test_arm"
    LIB_OUTDIR="/Users/bytedance/work/A1T1/output/android_armeabi_v7a"
    echo "=> Using NDK environment for Android arm (API $API_LEVEL)"
else
    echo "Error: Unsupported architecture: $ARCH"
    exit 1
fi

# 创建输出目录
mkdir -p "$OUTDIR"

# 编译 libAPF.so
if [ "$BUILD_LIB" == "yes" ]; then
    echo "=> Building libAPF.so for Android $ARCH..."
    cd ../../sorce/AudioProcessingAlgorithm
    ./build.sh all --ndk "$ARCH"
    cd -
fi

# 定义路径
ROOT_PATH="./../.."
AEC_INCLUDE="-I./../../sorce/AudioProcessingAlgorithm/audio_processing/include -I./../../sorce/AudioProcessingAlgorithm"


LDFLAGS="-L$LIB_OUTDIR"

# 编译参数（Android不需要-lpthread）
CFLAGS="-Wall -O3 -fPIC $AEC_INCLUDE"
LIBS="-ldl -lm -lAPF"

echo "=> Compiling: linux_test.cpp"
echo "   CC: $CC"
echo "   CXX: $CXX"
echo "   Output: $OUTDIR/$TARGET"

# 编译
$CXX $CFLAGS -o "$OUTDIR/$TARGET" linux_test.cpp $LDFLAGS $LIBS

# 复制运行库到输出目录
echo "=> Copying runtime libraries to $OUTDIR..."
cp "$LIB_OUTDIR/libAPF.so" "$OUTDIR/"
cp "$LIB_OUTDIR/libc++_shared.so" "$OUTDIR/"

echo "=> Compilation completed successfully!"
echo "   Output directory: $OUTDIR"
echo "   Files:"
echo "     - $OUTDIR/$TARGET (测试程序)"
echo "     - $OUTDIR/libAPF.so (音频处理库)"
echo "     - $OUTDIR/libc++_shared.so (NDK运行时库)"
echo ""
echo "To deploy to Android device:"
echo "   adb push $OUTDIR/ /data/local/tmp/android_test/"
echo "   adb shell chmod +x /data/local/tmp/android_test/$TARGET"
echo "   adb shell LD_LIBRARY_PATH=/data/local/tmp/android_test /data/local/tmp/android_test/$TARGET [args]"
