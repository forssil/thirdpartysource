#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "a1usbrecord/app_config.h"

struct pcm;

namespace a1usbrecord {

enum class PcmDirection {
    Capture,
    Playback,
};

class PcmDevice {
public:
    PcmDevice(PcmEndpoint endpoint, PcmDirection direction);
    ~PcmDevice();

    PcmDevice(const PcmDevice&) = delete;
    PcmDevice& operator=(const PcmDevice&) = delete;

    bool Open();
    void Close();

    bool IsOpen() const;
    bool ReadFrames(int16_t* buffer, std::size_t frames);
    bool WriteFrames(const int16_t* buffer, std::size_t frames);

    std::string LastError() const;
    const PcmEndpoint& Endpoint() const;

private:
    PcmEndpoint endpoint_;
    PcmDirection direction_;
    pcm* pcm_ = nullptr;
    std::string last_error_;
};

}  // namespace a1usbrecord
