# Audio SDK 编译说明

本文说明当前交付包中 `libAPF.so` 的编译方式。推荐入口为：

```text
sorce/AudioProcessingAlgorithm/build.sh
```

脚本会根据平台、ABI 和 Android API level 使用独立对象目录，并检查工具链、
NE10 静态库及最终产物。底层构建入口仍为：

```text
sorce/AudioProcessingAlgorithm/Makefile
```

直接调用 Makefile 时默认使用共享对象目录：

```text
objs/
```

因此手工切换平台编译前必须先执行 `clean`。通过 `build.sh` 编译时不需要：
脚本会设置独立的 `OBJ_PATH`。

## 推荐命令

Android ARM64 默认使用 API 28：

```bash
cd sorce/AudioProcessingAlgorithm
./build.sh all --ndk aarch64
```

常用的 Android ARM64 API 21 完整构建示例，同时生成 `libAPF.so` 和
`libAPF.a`：

```bash
cd /Users/bytedance/work/A1T1/thirdpartysource/trunk/sorce/AudioProcessingAlgorithm

./build.sh all \
  --ndk aarch64 \
  --api-level 21 \
  --ndk-path /Users/bytedance/work/A1T1/ndk/android-ndk-r28 \
  --objdir /Users/bytedance/work/A1T1/output/obj/android_arm64_v8a_api21 \
  --outdir /Users/bytedance/work/A1T1/output/android_arm64_v8a \
  --jobs 8
```

生成文件：

```text
/Users/bytedance/work/A1T1/output/android_arm64_v8a/libAPF.so
/Users/bytedance/work/A1T1/output/android_arm64_v8a/libAPF.a
/Users/bytedance/work/A1T1/output/android_arm64_v8a/libc++_shared.so
```

脚本会检查 `libAPF.so` 的 ELF 架构，并确认 NE10 已静态链接，不存在
`libNE10.so` 动态依赖。重复构建可以直接执行同一命令，不需要先 `clean`。

指定 NDK、API level 或只构建一种库：

```bash
./build.sh share --ndk aarch64 --api-level 21
./build.sh static --ndk aarch64 --ndk-path /opt/android-ndk-r28
```

ARM64 Mac 或 ARM64 Linux 上运行源码检查、精度测试和 benchmark：

```bash
./build.sh test
```

其他入口：

```bash
./build.sh all --ndk arm
./build.sh all --sigmastar
./build.sh all --sigmastar-docker
./build.sh clean --ndk aarch64
```

可通过 `--outdir`、`--objdir` 和 `--jobs` 覆盖输出目录、对象目录和并行数。
`--ndk-path` 的默认值依次来自 `ANDROID_NDK_HOME` 和工作区
`ndk/android-ndk-r28`。Android 构建直接传入 `libNE10.a`，并检查
`libAPF.so` 不依赖动态 `libNE10.so`。

## Android arm64-v8a

工具链：

```text
/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709
```

编译命令：

```bash
mkdir -p libs/android_arm64_v8a

make -C sorce/AudioProcessingAlgorithm clean \
  OUTDIR=/Users/duchengli/Workspace/trunk/libs/android_arm64_v8a

make -C sorce/AudioProcessingAlgorithm -j8 \
  OUTDIR=/Users/duchengli/Workspace/trunk/libs/android_arm64_v8a \
  CC=/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android28-clang \
  CXX=/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android28-clang++ \
  LD=/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/ld.lld \
  AR=/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-ar \
  'DEFINES=-fPIC -fpermissive -Wl,-rpath=.'
```

运行时库：

```bash
cp /Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/sysroot/usr/lib/aarch64-linux-android/libc++_shared.so \
  libs/android_arm64_v8a/libc++_shared.so
```

验证：

```bash
file libs/android_arm64_v8a/libAPF.so
```

期望结果包含：

```text
ELF 64-bit LSB shared object, ARM aarch64
```

## Android armeabi-v7a

编译命令：

```bash
mkdir -p libs/android_armeabi_v7a

make -C sorce/AudioProcessingAlgorithm clean \
  OUTDIR=/Users/duchengli/Workspace/trunk/libs/android_armeabi_v7a

make -C sorce/AudioProcessingAlgorithm -j8 \
  OUTDIR=/Users/duchengli/Workspace/trunk/libs/android_armeabi_v7a \
  CC=/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/armv7a-linux-androideabi28-clang \
  CXX=/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/armv7a-linux-androideabi28-clang++ \
  LD=/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/ld.lld \
  AR=/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/llvm-ar \
  'DEFINES=-fPIC -fpermissive -Wl,-rpath=.'
```

运行时库：

```bash
cp /Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/sysroot/usr/lib/arm-linux-androideabi/libc++_shared.so \
  libs/android_armeabi_v7a/libc++_shared.so
```

验证：

```bash
file libs/android_armeabi_v7a/libAPF.so
```

期望结果包含：

```text
ELF 32-bit LSB shared object, ARM, EABI5
```

## SigmaStar

SigmaStar 工具链是 Linux x86_64 ELF 程序，macOS 上不能直接执行。当前使用 `/Users/duchengli/Workspace/pulselink` 中的 Docker wrapper 编译。

工具链：

```text
/Users/duchengli/Workspace/pulselink/buildtools/sigmastar/arm-sigmastar-linux-uclibcgnueabihf-9.1.0
```

Docker wrapper：

```text
/Users/duchengli/Workspace/pulselink/scripts/sigmastar-docker
```

如果 Docker 镜像不存在，先执行：

```bash
cd /Users/duchengli/Workspace/pulselink
./scripts/sigmastar-docker build-image
```

编译命令：

```bash
cd /Users/duchengli/Workspace/pulselink

mkdir -p /Users/duchengli/Workspace/trunk/libs/sigmastar

./scripts/sigmastar-docker make /Users/duchengli/Workspace/trunk \
  -C sorce/AudioProcessingAlgorithm clean \
  OUTDIR=/work/libs/sigmastar

./scripts/sigmastar-docker make /Users/duchengli/Workspace/trunk \
  -C sorce/AudioProcessingAlgorithm -j8 \
  OUTDIR=/work/libs/sigmastar \
  'DEFINES=-fPIC -fpermissive -Wl,-rpath=.'
```

运行时库：

```bash
cp /Users/duchengli/Workspace/pulselink/buildtools/sigmastar/arm-sigmastar-linux-uclibcgnueabihf-9.1.0/arm-sigmastar-linux-uclibcgnueabihf/sysroot/lib/libgcc_s.so.1 \
  /Users/duchengli/Workspace/trunk/libs/sigmastar/libgcc_s.so.1

cp /Users/duchengli/Workspace/pulselink/buildtools/sigmastar/arm-sigmastar-linux-uclibcgnueabihf-9.1.0/arm-sigmastar-linux-uclibcgnueabihf/sysroot/lib/libstdc++.so.6.0.26 \
  /Users/duchengli/Workspace/trunk/libs/sigmastar/libstdc++.so.6
```

注意：SigmaStar 设备如果系统已经自带 `libstdc++.so.6`、`libgcc_s.so.1` 或 `libc.so.0`，不要覆盖系统库。附带 runtime 仅用于设备缺失或确认版本匹配时随应用目录部署。

验证：

```bash
file libs/sigmastar/libAPF.so
```

期望结果包含：

```text
ELF 32-bit LSB shared object, ARM, EABI5
```

更详细的 ELF 检查可在 Docker 中执行：

```bash
docker run --rm --platform linux/amd64 \
  -v /Users/duchengli/Workspace/pulselink/buildtools/sigmastar/arm-sigmastar-linux-uclibcgnueabihf-9.1.0:/opt/sigmastar:ro \
  -v /Users/duchengli/Workspace/trunk:/work \
  -w /work \
  sigmastar-uclibc:20.04-amd64 \
  bash -lc 'export PATH=/opt/sigmastar/bin:$PATH; arm-sigmastar-linux-uclibcgnueabihf-readelf -h libs/sigmastar/libAPF.so'
```

期望 ELF flags 包含：

```text
Version5 EABI, hard-float ABI
```

## 为什么覆盖 DEFINES

原始 Makefile 中默认 `DEFINES` 包含：

```text
--target=aarch64-linux-gnu -march=armv8-a
```

这组参数不适用于 Android 32 位和 SigmaStar。实际编译时统一用：

```text
DEFINES=-fPIC -fpermissive -Wl,-rpath=.
```

目标平台由对应编译器 wrapper 决定：

```text
aarch64-linux-android28-clang++
armv7a-linux-androideabi28-clang++
arm-sigmastar-linux-uclibcgnueabihf-g++
```

## Sample 链接验证

Android arm64：

```bash
/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/aarch64-linux-android28-clang++ \
  -Irelease/audio_aec/include \
  release/audio_aec/sample/linux_test.cpp \
  -Llibs/android_arm64_v8a \
  -lAPF \
  -Wl,-rpath=. \
  -o /tmp/audio_aec_sample_arm64
```

Android armeabi-v7a：

```bash
/Users/duchengli/Library/Android/sdk/ndk/28.1.13356709/toolchains/llvm/prebuilt/darwin-x86_64/bin/armv7a-linux-androideabi28-clang++ \
  -Irelease/audio_aec/include \
  release/audio_aec/sample/linux_test.cpp \
  -Llibs/android_armeabi_v7a \
  -lAPF \
  -Wl,-rpath=. \
  -o /tmp/audio_aec_sample_armeabi_v7a
```

SigmaStar：

```bash
docker run --rm --platform linux/amd64 \
  -v /Users/duchengli/Workspace/pulselink/buildtools/sigmastar/arm-sigmastar-linux-uclibcgnueabihf-9.1.0:/opt/sigmastar:ro \
  -v /Users/duchengli/Workspace/trunk:/work \
  -w /work \
  sigmastar-uclibc:20.04-amd64 \
  bash -lc 'export PATH=/opt/sigmastar/bin:$PATH; arm-sigmastar-linux-uclibcgnueabihf-g++ -Irelease/audio_aec/include release/audio_aec/sample/linux_test.cpp -Llibs/sigmastar -lAPF -lpthread -Wl,-rpath=. -o /tmp/audio_aec_sample_sigmastar'
```

## 常见 warning

当前源码编译时会出现一些 warning，例如：

- 未使用变量。
- 构造函数初始化顺序。
- 部分 `.c` 文件被 C++ 编译器编译。
- 编译阶段传入 `-Wl,-rpath=.` 被提示为 linker input unused。

这些 warning 当前未阻止 `libAPF.so` 生成。若要作为长期 SDK 维护，建议后续清理 Makefile 中 C/C++ 编译规则和源码 warning。
