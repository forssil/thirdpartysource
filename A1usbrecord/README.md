# A1usbrecord

## 项目任务

Android UAC2 多声道采集设备开发：开发一个类似 `av_virtual` 的 native 进程，
将本地两路 ALSA 采集源合成为 10 声道音频，并写入 UAC2 gadget，使 PC 端能够以
USB Audio Class 2.0 设备采集到 10 channel PCM 数据。

## 目标

- 目标声道数：8+2 声道，共 10 channel
- 采样率：48000 Hz
- 采样格式：S16_LE
- PC 端目标：通过 UAC2 采集到 10 channel 音频数据

## 输入设备

- `card0 / pcmC0D0c`
  - 名称：`multicodec8ch`
  - 功能：本地 8 声道采集源
  - 当前参数：8ch / 48000 Hz / S16_LE
- `card1 / pcmC1D0c`
  - 名称：`rockchip-acm8625p`
  - 功能：本地 2 声道 I2S 采集源，可能用于辅助输入、参考音或 Line-In 路径
  - 当前参数：2ch / 48000 Hz / S16_LE

## 输出设备

- `card4 / UAC2_Gadget`
  - UAC2 gadget 已启用
  - PC 录音设备通道数由 `p_chmask` 控制
  - 设备侧写入方向为 `pcmC4D0p`
  - 当前 10ch 验证参数：`p_chmask=0x3ff`、`p_srate=48000`、`p_ssize=2`

## 初步数据流

```text
pcmC0D0c 8ch capture  ┐
                      ├─ A1usbrecord native 进程 ── 10ch interleaved PCM ── UAC2 gadget ── PC capture
pcmC1D0c 2ch capture  ┘
```

## 关键技术点

- 同时打开并读取 `pcmC0D0c` 与 `pcmC1D0c`
- 将 8ch + 2ch 合成为 10ch interleaved PCM
- 写入 UAC2 gadget，使 PC 端枚举并采集 10 channel
- 修改 UAC2 configfs/init 配置中的 `p_chmask`、`p_srate`、`p_ssize`
- 处理 `card0` 与 `card1` 之间可能存在的时钟漂移、读写阻塞和缓冲区水位问题
- 避免与系统现有 `/vendor/bin/av_virtual` 同时抢占 ALSA PCM 设备

## 工程结构

```text
A1usbrecord/
  CMakeLists.txt
  include/a1usbrecord/
    app_config.h
    audio_muxer.h
    pcm_device.h
  src/
    main.cpp
    audio_muxer.cpp
    pcm_device.cpp
  docs/
    device_map.md
    implementation_plan.md
```

## 当前实现状态

- 已建立 native C++ 工程骨架
- 编译产物名定义为 `av_virtual`，用于替换 `/vendor/bin/av_virtual` 后复用现有 init service 自动启动
- 已固定默认设备配置：
  - `card0/device0`：8ch capture，period size `1024`
  - `card1/device0`：2ch capture，period size `1024`
  - `card4/device0`：10ch UAC2 输出，period size 使用已验证可用的 `192`
- 已实现 8ch + 2ch 到 10ch interleaved PCM 的基础 muxer
- 已改为三线程结构：
  - `card0` 采集线程：阻塞读取 1024 帧并推入队列
  - `card1` 采集线程：阻塞读取 1024 帧并推入队列
  - mux/write 线程：合成 10ch 后按 192 帧分块写入 `pcmC4D0p`
- 程序默认直接运行，便于替换 `/vendor/bin/av_virtual` 后由现有 init service 自动启动
- 手动检查配置时可传入 `--dry-run`，此模式只打印配置，不打开 PCM 设备

## 构建说明

当前工程使用 CMake 描述 native 可执行程序：

```bash
cmake -S . -B build
cmake --build build
```

编译产物名：

```text
av_virtual
```

目标部署路径：

```text
/vendor/bin/av_virtual
```

运行方式：

```bash
./av_virtual
./av_virtual --dry-run
```

实际在 Android 目标环境构建时，需要确保 tinyalsa 头文件和 `libtinyalsa` 链接路径可用。

## 参考文档

- `d:\hardware\A1T1\Android UAC2 多声道采集设备开发总结.pdf`
