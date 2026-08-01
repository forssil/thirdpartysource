# A1usbrecord Device Map

本文档记录当前 RK3588 Android 设备上已确认的音频设备映射。

## 输入设备

### card0 / pcmC0D0c

- ALSA 名称：`multicodec8ch`
- PCM 节点：`/dev/snd/pcmC0D0c`
- tinyalsa 参数：`-D 0 -d 0`
- 方向：capture
- 当前参数：8ch / 48000 Hz / S16_LE
- 当前占用：`/vendor/bin/av_virtual`
- 任务角色：主 8 声道采集源

### card1 / pcmC1D0c

- ALSA 名称：`rockchip-acm8625p`
- PCM 节点：`/dev/snd/pcmC1D0c`
- tinyalsa 参数：`-D 1 -d 0`
- 方向：capture
- 当前参数：2ch / 48000 Hz / S16_LE
- 当前占用：`/vendor/bin/av_virtual`
- 任务角色：额外 2 声道采集源，可能用于辅助输入、参考音或 Line-In 路径

### card1 / pcmC1D0p

- ALSA 名称：`rockchip-acm8625p`
- PCM 节点：`/dev/snd/pcmC1D0p`
- tinyalsa 参数：`-D 1 -d 0`
- 方向：playback
- 当前参数：2ch / 48000 Hz / S16_LE
- 任务角色：PC 播放方向的本地输出设备

## UAC2 设备

### card4 / UAC2_Gadget

- ALSA 名称：`UAC2_Gadget`
- capture 节点：`/dev/snd/pcmC4D0c`
- playback 节点：`/dev/snd/pcmC4D0p`
- tinyalsa 参数：`-D 4 -d 0`
- 当前 PC 录音方向参数：10ch / 48000 Hz / S16_LE
- 当前 PC 播放方向参数：2ch / 48000 Hz / S16_LE
- 当前占用：`/vendor/bin/av_virtual`
- 任务角色：向 PC 暴露 USB Audio Class 2.0 录音设备，并接收 PC 播放音频

方向关系：

```text
p_chmask -> PC 录音设备通道数 -> Android 侧写 pcmC4D0p
c_chmask -> PC 播放设备通道数 -> Android 侧读 pcmC4D0c
```

## 当前 UAC2 配置源

UAC2 configfs 参数由以下 init 文件写入：

```text
/vendor/etc/init/hw/init.rk30board.usb.rc
```

当前关键参数：

```text
p_chmask = 0x3ff
p_srate  = 48000
p_ssize  = 2
c_chmask = 0x3
c_srate  = 48000
c_ssize  = 2
```

`p_chmask = 0x3ff` 表示 PC 录音方向 10 channel。该配置已通过 10ch/48k/S16_LE WAV 写入 `pcmC4D0p` 验证。
`c_chmask = 0x3` 表示 PC 播放方向 2 channel。当前工程从 `pcmC4D0c` 读取该方向数据并写入 `pcmC1D0p`。
