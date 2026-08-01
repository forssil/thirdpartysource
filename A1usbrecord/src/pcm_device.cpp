#include "a1usbrecord/pcm_device.h"

#if A1USBRECORD_HAS_TINYALSA
#include <cerrno>
#include <cstring>
#include <poll.h>

#include <tinyalsa/asoundlib.h>

extern "C" int pcm_get_poll_fd(struct pcm* pcm);
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

bool PcmDevice::WaitForReadable(int timeout_ms, bool* readable) {
    if (readable == nullptr) {
        last_error_ = "readable output pointer is null";
        return false;
    }
    *readable = false;

#if A1USBRECORD_HAS_TINYALSA
    if (pcm_ == nullptr) {
        last_error_ = "capture device is not open";
        return false;
    }

    const int fd = pcm_get_poll_fd(pcm_);
    if (fd < 0) {
        last_error_ = "failed to get PCM poll fd";
        return false;
    }

    struct pollfd poll_fd {};
    poll_fd.fd = fd;
    poll_fd.events = POLLIN;

    const int ret = poll(&poll_fd, 1, timeout_ms);
    if (ret < 0) {
        last_error_ = std::strerror(errno);
        return false;
    }
    if (ret == 0) {
        return true;
    }
    if ((poll_fd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
        last_error_ = "PCM poll returned error event";
        return false;
    }

    *readable = (poll_fd.revents & POLLIN) != 0;
    return true;
#else
    (void)timeout_ms;
    last_error_ = "tinyalsa support is not enabled in this build";
    return false;
#endif
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
