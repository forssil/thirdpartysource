#pragma once

#include <cstdint>

namespace a1usbrecord {

struct PcmEndpoint {
    unsigned int card = 0;
    unsigned int device = 0;
    unsigned int channels = 0;
    unsigned int rate = 48000;
    unsigned int bits = 16;
    unsigned int period_size = 480;
    unsigned int period_count = 4;
};

struct AppConfig {
    PcmEndpoint input8ch{0, 0, 8, 48000, 16, 1024, 4};
    PcmEndpoint input2ch{1, 0, 2, 48000, 16, 1024, 4};
    PcmEndpoint uac2Output{4, 0, 10, 48000, 16, 192, 4};
    PcmEndpoint uac2Input{4, 0, 2, 48000, 16, 192, 4};
    PcmEndpoint localPlayback{1, 0, 2, 48000, 16, 192, 4};

    const char* logDir = "/data/vendor/av_virtual";
    const char* logPath = "/data/vendor/av_virtual/av_virtual.log";
    unsigned int statsLogIntervalSeconds = 300;
    unsigned int playbackQueueChunkFrames = 256;
    int uacInputPollTimeoutMs = 10;
    float uacPlaybackGain = 0.5f;
    bool dry_run = true;
};

constexpr unsigned int kMergedChannels = 10;
constexpr unsigned int kInput8Channels = 8;
constexpr unsigned int kInput2Channels = 2;

}  // namespace a1usbrecord
