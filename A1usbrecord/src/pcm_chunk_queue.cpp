#include "a1usbrecord/pcm_chunk_queue.h"

#include <utility>

namespace a1usbrecord {

PcmChunkQueue::PcmChunkQueue(std::size_t capacity) : capacity_(capacity) {}

bool PcmChunkQueue::Push(PcmChunk chunk) {
    std::unique_lock<std::mutex> lock(mutex_);
    can_push_.wait(lock, [this]() {
        return stopped_ || chunks_.size() < capacity_;
    });

    if (stopped_) {
        return false;
    }

    chunks_.push_back(std::move(chunk));
    can_pop_.notify_one();
    return true;
}

bool PcmChunkQueue::Pop(PcmChunk* chunk) {
    if (chunk == nullptr) {
        return false;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    can_pop_.wait(lock, [this]() {
        return stopped_ || !chunks_.empty();
    });

    if (chunks_.empty()) {
        return false;
    }

    *chunk = std::move(chunks_.front());
    chunks_.pop_front();
    can_push_.notify_one();
    return true;
}

void PcmChunkQueue::Stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }
    can_push_.notify_all();
    can_pop_.notify_all();
}

std::size_t PcmChunkQueue::Size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return chunks_.size();
}

std::size_t PcmChunkQueue::Capacity() const {
    return capacity_;
}

}  // namespace a1usbrecord
