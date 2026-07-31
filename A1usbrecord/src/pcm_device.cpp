#include "a1usbrecord/pcm_device.h"

#if A1USBRECORD_HAS_TINYALSA
#include <tinyalsa/asoundlib.h>
#endif

namespace a1usbrecord {

#if A1USBRECORD_HAS_TINYALSA
namespace {

struct pcm_config ToTinyalsaConfig(const PcmEndpoint& endpoint) {
    struct pcm_config config {};
    config.channels = endpoint.channels;
    config.rate = endpoint.rate;
    config.period_size = endpoint.period_size;
    config.period_count = endpoint.period_count;
    config.format = PCM_FORMAT_S16_LE;
    config.start_threshold = 0;
    config.stop_threshold = 0;
    config.silence_threshold = 0;
    return config;
}

unsigned int ToTinyalsaFlags(PcmDirection direction) {
    return direction == PcmDirection::Capture ? PCM_IN : PCM_OUT;
}

unsigned int FramesToBytes(const PcmEndpoint& endpoint, std::size_t frames) {
    return static_cast<unsigned int>(frames * endpoint.channels * (endpoint.bits / 8));
}

}  // namespace
#endif

PcmDevice::PcmDevice(PcmEndpoint endpoint, PcmDirection direction)
    : endpoint_(endpoint), direction_(direction) {}

PcmDevice::~PcmDevice() {
    Close();
}

bool PcmDevice::Open() {
#if A1USBRECORD_HAS_TINYALSA
    Close();

    struct pcm_config config = ToTinyalsaConfig(endpoint_);
    pcm_ = pcm_open(endpoint_.card, endpoint_.device, ToTinyalsaFlags(direction_), &config);
    if (pcm_ == nullptr) {
        last_error_ = "pcm_open returned null";
        return false;
    }

    if (!pcm_is_ready(pcm_)) {
        last_error_ = pcm_get_error(pcm_);
        Close();
        return false;
    }

    last_error_.clear();
    return true;
#else
    last_error_ = "tinyalsa support is not enabled in this build";
    return false;
#endif
}

void PcmDevice::Close() {
#if A1USBRECORD_HAS_TINYALSA
    if (pcm_ != nullptr) {
        pcm_close(pcm_);
        pcm_ = nullptr;
    }
#else
    pcm_ = nullptr;
#endif
}

bool PcmDevice::IsOpen() const {
    return pcm_ != nullptr;
}

bool PcmDevice::ReadFrames(int16_t* buffer, std::size_t frames) {
#if A1USBRECORD_HAS_TINYALSA
    if (pcm_ == nullptr || buffer == nullptr) {
        last_error_ = "capture device is not open";
        return false;
    }

    if (pcm_read(pcm_, buffer, FramesToBytes(endpoint_, frames)) < 0) {
        last_error_ = pcm_get_error(pcm_);
        return false;
    }

    return true;
#else
    (void)buffer;
    (void)frames;
    last_error_ = "tinyalsa support is not enabled in this build";
    return false;
#endif
}

bool PcmDevice::WriteFrames(const int16_t* buffer, std::size_t frames) {
#if A1USBRECORD_HAS_TINYALSA
    if (pcm_ == nullptr || buffer == nullptr) {
        last_error_ = "playback device is not open";
        return false;
    }

    if (pcm_write(pcm_, buffer, FramesToBytes(endpoint_, frames)) < 0) {
        last_error_ = pcm_get_error(pcm_);
        return false;
    }

    return true;
#else
    (void)buffer;
    (void)frames;
    last_error_ = "tinyalsa support is not enabled in this build";
    return false;
#endif
}

std::string PcmDevice::LastError() const {
    return last_error_;
}

const PcmEndpoint& PcmDevice::Endpoint() const {
    return endpoint_;
}

}  // namespace a1usbrecord
