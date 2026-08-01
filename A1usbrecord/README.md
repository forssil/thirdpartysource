# A1usbrecord

## 项目任务

Android UAC2 多声道采集设备开发：开发一个类似 `av_virtual` 的 native 进程，
将本地两路 ALSA 采集源合成为 10 声道音频，并写入 UAC2 gadget，使 PC 端能够以
USB Audio Class 2.0 设备采集到 10 channel PCM 数据。同时转发 PC 端播放音频到
本地播放设备。

## 目标

- 目标声道数：8+2 声道，共 10 channel
- 采样率：48000 Hz
- 采样格式：S16_LE
- PC 端目标：通过 UAC2 采集到 10 channel 音频数据
- 设备播放目标：PC 播放 2ch/48k/S16_LE 音频时，Android 侧从 UAC2 读取并写入本地播放设备

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
- `card1 / pcmC1D0p`
  - 名称：`rockchip-acm8625p`
  - 功能：本地 2 声道播放输出
  - 当前参数：2ch / 48000 Hz / S16_LE

## 初步数据流

```text
pcmC0D0c 8ch capture  ┐
                      ├─ A1usbrecord native 进程 ── 10ch interleaved PCM ── UAC2 gadget ── PC capture
pcmC1D0c 2ch capture  ┘

PC playback ── UAC2 gadget ── pcmC4D0c 2ch capture ── A1usbrecord native 进程 ── pcmC1D0p 2ch playback
```

## 关键技术点

- 同时打开并读取 `pcmC0D0c` 与 `pcmC1D0c`
- 将 8ch + 2ch 合成为 10ch interleaved PCM
- 写入 UAC2 gadget，使 PC 端枚举并采集 10 channel
- 从 UAC2 gadget 读取 PC 播放方向 2ch PCM，并写入本地播放设备
- 修改 UAC2 configfs/init 配置中的 `p_chmask`、`p_srate`、`p_ssize`
- PC 播放方向由 `c_chmask`、`c_srate`、`c_ssize` 控制，当前按 2ch/48k/S16_LE 处理
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
- 已新增 PC 播放转发线程：
  - 从 `pcmC4D0c` 阻塞读取 2ch/48k/S16_LE
  - 使用 256 帧连接 buffer 转发播放数据
  - 写入 `pcmC1D0p` 本地播放输出
- 已新增本地文件日志：
  - 默认路径：`/data/local/tmp/av_virtual.log`
  - `FileLogger` 使用独立线程异步落盘，采集/播放线程只投递日志消息
  - 默认每 5 分钟记录采集帧数、播放帧数、采集两路帧数差、UAC2 输出滞后、播放读写差、队列深度和错误计数
- 程序默认直接运行，便于替换 `/vendor/bin/av_virtual` 后由现有 init service 自动启动
- 手动检查配置时可传入 `--dry-run`，此模式只打印配置，不打开 PCM 设备

## av_virtual 修改方案

本工程的部署方式是生成新的 `av_virtual` 可执行文件，替换系统原有：

```text
/vendor/bin/av_virtual
```

继续复用原有 init service：

```rc
service vendor.av_virtual /vendor/bin/av_virtual
    class late_start
    user root
    group root audio camera media inet
    disabled
    restart_period 5
```

启动触发仍使用原系统逻辑：

```rc
on property:vendor.all.modules.ready=1
    start vendor.av_virtual
```

### 功能变化

- 原 `av_virtual` 的实现被替换为 UAC2 音频桥接进程。
- PC 录音方向：
  - 读取 `pcmC0D0c` 的 8ch 本地采集。
  - 读取 `pcmC1D0c` 的 2ch 本地采集。
  - 合成为 10ch interleaved S16_LE。
  - 写入 `pcmC4D0p`，供 PC 端作为 UAC2 录音设备采集。
- PC 播放方向：
  - 从 `pcmC4D0c` 读取 PC 下发的 2ch 播放音频。
  - 使用 256 帧 buffer 在 UAC2 capture PCM 和本地播放 PCM 之间转发。
  - 写入 `pcmC1D0p`，输出到本地播放设备。
- 稳定性日志：
  - 写入 `/data/local/tmp/av_virtual.log`。
  - 由独立日志线程异步写入，避免文件 I/O 阻塞音频采集和播放线程。
  - 默认每 5 分钟记录采集偏差、播放偏差、UAC2 写入滞后、队列水位和错误计数。

### UAC2 配置要求

PC 录音 10ch 依赖 init 中的 UAC2 playback 参数：

```rc
write /config/usb_gadget/g1/functions/uac2.gs0/p_chmask 0x3ff
write /config/usb_gadget/g1/functions/uac2.gs0/p_srate 48000
write /config/usb_gadget/g1/functions/uac2.gs0/p_ssize 2
```

PC 播放 2ch 依赖 UAC2 capture 参数：

```rc
write /config/usb_gadget/g1/functions/uac2.gs0/c_chmask 0x3
write /config/usb_gadget/g1/functions/uac2.gs0/c_srate 48000
write /config/usb_gadget/g1/functions/uac2.gs0/c_ssize 2
```

方向关系：

```text
p_chmask -> PC 录音设备通道数 -> Android 侧写 pcmC4D0p
c_chmask -> PC 播放设备通道数 -> Android 侧读 pcmC4D0c
```

### 部署注意事项

替换 `/vendor/bin/av_virtual` 后需要确认权限和 SELinux label：

```sh
chmod 755 /vendor/bin/av_virtual
chown root:shell /vendor/bin/av_virtual
restorecon /vendor/bin/av_virtual
ls -lZ /vendor/bin/av_virtual
```

期望 label：

```text
u:object_r:av_virtual_exec:s0
```

如果 `restorecon` 后 label 不正确，需要恢复为：

```sh
chcon u:object_r:av_virtual_exec:s0 /vendor/bin/av_virtual
```

启动和检查：

```sh
start vendor.av_virtual
getprop init.svc.vendor.av_virtual
getprop init.svc_debug_pid.vendor.av_virtual
tail -f /data/local/tmp/av_virtual.log
```

回滚方式是恢复原始 `/vendor/bin/av_virtual`，并重新执行权限和 label 修复。

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

查看稳定性日志：

```bash
tail -f /data/local/tmp/av_virtual.log
```

实际在 Android 目标环境构建时，需要确保 tinyalsa 头文件和 `libtinyalsa` 链接路径可用。

## 参考文档

- `d:\hardware\A1T1\Android UAC2 多声道采集设备开发总结.pdf`
