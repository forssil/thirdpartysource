#!/bin/bash

# 发生错误时立即退出
set -e

# 获取脚本所在目录，确保在 Makefile 所在目录下执行
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# 打印帮助信息
function show_help() {
    echo "Usage: $0 [COMMAND] [OPTIONS]"
    echo ""
    echo "Commands:"
    echo "  all       (Default) 编译整个项目"
    echo "  clean     清理编译产物 (objs 和 libs)"
    echo ""
    echo "Options for 'all':"
    echo "  -c, --cross-compile <prefix>  指定自定义的交叉编译工具链前缀"
    echo "                                例如: $0 all -c /opt/toolchain/bin/arm-linux-"
    echo "  -o, --outdir <path>           指定输出文件夹路径 (静态库将生成在此处)"
    echo "  --cc <compiler>               指定 C 编译器 (例如: gcc, clang)"
    echo "  --cxx <compiler>              指定 C++ 编译器 (例如: g++, clang++)"
    echo "  --ld <linker>                 指定链接器 (例如: ld, ld.lld)"
    echo "  --ar <archiver>               指定静态库打包工具 (例如: ar, llvm-ar)"
    echo "  --ndk <arch>                  使用本地 Android NDK 环境编译，支持的架构: aarch64, arm"
    echo "                                例如: $0 all --ndk aarch64"
    echo "  --sigmastar                   使用 SigmaStar 交叉编译工具链编译"
    echo ""
    echo "Examples:"
    echo "  $0                      # 使用 Makefile 中的默认工具链编译"
    echo "  $0 clean                # 清理"
    echo "  $0 all --cc gcc         # 仅覆盖 CC 编译器为系统 gcc"
    echo "  $0 all -c aarch64-linux-gnu- # 使用指定的交叉编译工具链前缀"
    echo "  $0 all -o /tmp/build_libs    # 指定输出目录"
    echo "  $0 all --ndk aarch64         # 使用本地下载的 NDK 编译 Android aarch64 版本"
    echo "  $0 all --ndk arm             # 使用本地下载的 NDK 编译 Android 32位 arm 版本"
    echo "  $0 all --sigmastar           # 使用 SigmaStar 工具链编译"
    echo "  $0 all -c /Users/bytedance/work/A1T1/armbuild/bin/arm-sigmastar-linux-uclibcgnueabihf-9.1.0- -o /Users/bytedance/work/A1T1/output/arm"
}

# 默认执行目标
TARGET="all"
MAKE_ARGS=()
NDK_RUNTIME_COPY=""
AUTO_CLEAN=""

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

# 解析其余参数（处理外部传入的编译器变量）
while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--cross-compile)
            export CROSS_COMPILE="$2"
            echo "=> Set CROSS_COMPILE to: $CROSS_COMPILE"
            shift 2
            ;;
        -o|--outdir)
            export OUTDIR="$2"
            echo "=> Set OUTDIR to: $OUTDIR"
            shift 2
            ;;
        --cc)
            export CC="$2"
            echo "=> Set CC to: $CC"
            shift 2
            ;;
        --cxx)
            export CXX="$2"
            echo "=> Set CXX to: $CXX"
            shift 2
            ;;
        --ld)
            export LD="$2"
            echo "=> Set LD to: $LD"
            shift 2
            ;;
        --ar)
            export AR="$2"
            echo "=> Set AR to: $AR"
            shift 2
            ;;
        --ndk)
            NDK_BIN="/Users/bytedance/work/A1T1/ndk/android-ndk-r28/toolchains/llvm/prebuilt/darwin-x86_64/bin"
            NDK_ARCH="$2"
            
            if [ "$NDK_ARCH" == "aarch64" ]; then
                export CC="$NDK_BIN/aarch64-linux-android28-clang"
                export CXX="$NDK_BIN/aarch64-linux-android28-clang++"
                DEFAULT_NDK_OUTDIR="/Users/bytedance/work/A1T1/output/android_arm64_v8a"
                NDK_RUNTIME_COPY="aarch64-linux-android"
                echo "=> Using NDK environment for Android aarch64 (API 28)"
            elif [ "$NDK_ARCH" == "arm" ]; then
                export CC="$NDK_BIN/armv7a-linux-androideabi28-clang"
                export CXX="$NDK_BIN/armv7a-linux-androideabi28-clang++"
                DEFAULT_NDK_OUTDIR="/Users/bytedance/work/A1T1/output/android_armeabi_v7a"
                NDK_RUNTIME_COPY="arm-linux-androideabi"
                echo "=> Using NDK environment for Android arm (API 28)"
            else
                echo "Error: Invalid architecture for --ndk. Supported values are: aarch64, arm"
                exit 1
            fi
            
            export LD="$NDK_BIN/ld.lld"
            export AR="$NDK_BIN/llvm-ar"
            
            # 只有当用户没有显式传入 OUTDIR 时，才使用 NDK 的默认输出目录
            if [ -z "$OUTDIR" ]; then
                export OUTDIR="$DEFAULT_NDK_OUTDIR"
            fi
            
            echo "   CC: $CC"
            echo "   CXX: $CXX"
            echo "   LD: $LD"
            echo "   AR: $AR"
            echo "   OUTDIR: $OUTDIR"
            AUTO_CLEAN="yes"
            shift 2
            ;;
        --sigmastar)
            SIGMA_BIN="/Users/bytedance/work/A1T1/armbuild/bin/arm-sigmastar-linux-uclibcgnueabihf-9.1.0-"
            export CC="${SIGMA_BIN}gcc"
            export CXX="${SIGMA_BIN}g++"
            export LD="${SIGMA_BIN}ld"
            export AR="${SIGMA_BIN}ar"
            DEFAULT_SIGMA_OUTDIR="/Users/bytedance/work/A1T1/output/sigmastar"
            
            if [ -z "$OUTDIR" ]; then
                export OUTDIR="$DEFAULT_SIGMA_OUTDIR"
            fi
            
            echo "=> Using SigmaStar cross-compile toolchain"
            echo "   CC: $CC"
            echo "   CXX: $CXX"
            echo "   LD: $LD"
            echo "   AR: $AR"
            echo "   OUTDIR: $OUTDIR"
            AUTO_CLEAN="yes"
            shift 1
            ;;
        *)
            echo "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
done

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
if [ "$TARGET" == "clean" ]; then
    echo "=> Running: make clean"
    make clean
else
    # 切换平台编译前先 clean，避免复用其它平台的 .o 文件
    if [ -n "$AUTO_CLEAN" ]; then
        echo "=> Running: make clean (auto clean before cross-compile)"
        make clean
    fi
    echo "=> Running: make -j${CORES} ${TARGET} ${MAKE_ARGS[*]}"
    make -j"${CORES}" "$TARGET" "${MAKE_ARGS[@]}"

    # 拷贝 NDK 运行时库 libc++_shared.so 到输出目录
    if [ -n "$NDK_RUNTIME_COPY" ] && [ -n "$OUTDIR" ]; then
        NDK_SYSROOT="/Users/bytedance/work/A1T1/ndk/android-ndk-r28/toolchains/llvm/prebuilt/darwin-x86_64/sysroot/usr/lib/${NDK_RUNTIME_COPY}/libc++_shared.so"
        if [ -f "$NDK_SYSROOT" ]; then
            cp "$NDK_SYSROOT" "$OUTDIR/"
            echo "=> Copied libc++_shared.so to $OUTDIR"
        else
            echo "Warning: libc++_shared.so not found at $NDK_SYSROOT"
        fi
    fi
fi
echo "---------------------------------------------------"
echo "Done!"


#./build.sh all  -c /Users/bytedance/work/A1T1/clangbuild/bin/llvm- -o /Users/bytedance/work/A1T1/output/android
#./build.sh all -c /Users/bytedance/work/A1T1/armbuild/bin/arm-sigmastar-linux-uclibcgnueabihf-9.1.0- -o /Users/bytedance/work/A1T1/output/arm