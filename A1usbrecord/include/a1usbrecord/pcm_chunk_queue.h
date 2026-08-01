#pragma once

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <vector>

namespace a1usbrecord {

struct PcmChunk {
    std::size_t frames = 0;
    unsigned int channels = 0;
    std::vector<int16_t> samples;
};

class PcmChunkQueue {
public:
    explicit PcmChunkQueue(std::size_t capacity);

    bool Push(PcmChunk chunk);
    bool Pop(PcmChunk* chunk);
    bool TryPop(PcmChunk* chunk);
    void Stop();
    std::size_t Size() const;
    std::size_t Capacity() const;

private:
    std::size_t capacity_;
    bool stopped_ = false;
    mutable std::mutex mutex_;
    std::condition_variable can_push_;
    std::condition_variable can_pop_;
    std::deque<PcmChunk> chunks_;
};

}  // namespace a1usbrecord
