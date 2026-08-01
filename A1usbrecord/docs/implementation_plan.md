# A1usbrecord Implementation Plan

## 阶段 1：工程骨架

- 建立 native 工程目录。
- 编译产物名定义为 `av_virtual`，用于替换 `/vendor/bin/av_virtual` 后复用现有 `vendor.av_virtual` init service 自动启动。
- 固定 card0/card1/UAC2 的默认设备配置。
- 提供 8ch + 2ch 到 10ch interleaved PCM 的 muxer。
- 程序默认直接运行，手动检查时通过 `--dry-run` 只打印配置。

## 阶段 2：设备独占验证

- 临时停止或绕开 `/vendor/bin/av_virtual`。
- 使用 tinyalsa 验证 `pcmC0D0c` 可独占读取 8ch。
- 使用 tinyalsa 验证 `pcmC1D0c` 可独占读取 2ch。
- 验证设备侧写入 UAC2 的方向，重点确认 `pcmC4D0p` 是否对应 PC 端 capture。

## 阶段 3：UAC2 10 channel 配置

- 修改 UAC2 configfs/init 中的 `p_chmask`、`p_srate`、`p_ssize`。
- 重新枚举 USB gadget。
- 在 PC 端确认 USB Audio 录音设备显示为 10 channel capture。
- 验证 Linux、Windows 或目标 PC 软件对 10 channel UAC2 的兼容性。

方向关系：

```text
p_chmask -> PC 录音设备通道数 -> Android 侧写 pcmC4D0p
c_chmask -> PC 播放设备通道数 -> Android 侧读 pcmC4D0c
```

## 阶段 4：桥接进程实现

- 同时打开 card0 8ch capture 和 card1 2ch capture。
- 使用三线程结构：
  - card0 capture thread：按 1024 帧阻塞读取 8ch PCM，推入队列。
  - card1 capture thread：按 1024 帧阻塞读取 2ch PCM，推入队列。
  - mux/write thread：从两个队列各取一块，合成后按 UAC2 支持的 192 帧分块写入。
- 增加 PC 播放转发线程：
  - 从 UAC2 capture PCM `pcmC4D0c` 读取 PC 播放方向 2ch/48k/S16_LE。
  - 使用 256 帧连接 buffer 转发播放数据。
  - 写入本地播放设备 `pcmC1D0p`。
- 合成为 10ch interleaved PCM：

```text
out[0..7] = card0 ch0..ch7
out[8..9] = card1 ch0..ch1
```

- 写入 UAC2 gadget。
- 增加退出信号处理、错误日志和统计信息。
- 本地日志写入 `/data/vendor/av_virtual/av_virtual.log`，由独立日志线程异步落盘，避免文件 I/O 阻塞音频线程。默认每 5 分钟记录采集和播放链路偏差：
  - `capture_diff_frames`：card0 与 card1 累计采集帧数差。
  - `uac2_lag_frames`：已 mux 帧数与已写入 UAC2 帧数差。
  - `playback_diff_frames`：从 UAC2 读取的 PC 播放帧数与本地播放写入帧数差。
  - `q8` / `q2`：两路采集队列水位。
- 部署时替换 `/vendor/bin/av_virtual`，由现有 `vendor.av_virtual` service 自动拉起。

## 阶段 5：稳定性处理

- 增加 ring buffer 和水位统计。
- 监控 tinyalsa `pcm_read` / `pcm_write` 阻塞和 XRUN。
- 处理 card0 与 card1 之间可能存在的时钟漂移。
- 必要时对 2ch 输入做轻量丢补帧或重采样。

## 当前风险

- `av_virtual` 当前占用 card0、card1 和 UAC2，测试前需要处理占用。
- UAC2 PC 录音方向需要固化 `p_chmask=0x3ff` 后重启验证。
- 10 channel channel mask 需要结合 Linux UAC2 gadget 和 PC 端驱动实际验证。
- card0 和 card1 未确认是否共享同一音频时钟。
