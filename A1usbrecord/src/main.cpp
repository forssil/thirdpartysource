#include <atomic>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <utility>
#include <vector>

#include "a1usbrecord/app_config.h"
#include "a1usbrecord/audio_muxer.h"
#include "a1usbrecord/pcm_chunk_queue.h"
#include "a1usbrecord/pcm_device.h"

namespace {

// Asynchronous file logger. Audio threads only enqueue log lines; the worker
// thread performs file I/O so capture/playback timing is not blocked by storage.
class FileLogger {
public:
    explicit FileLogger(const char* path) : stream_(path, std::ios::app) {
        worker_ = std::thread(&FileLogger::WorkerLoop, this);
    }

    ~FileLogger() {
        Stop();
    }

    FileLogger(const FileLogger&) = delete;
    FileLogger& operator=(const FileLogger&) = delete;

    void Log(const std::string& message) {
        const auto now = std::chrono::system_clock::now().time_since_epoch();
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) {
                return;
            }

            // Timestamp is created before enqueue so the log time reflects the
            // producing thread event, not the later file write time.
            std::ostringstream line;
            line << '[' << ms << "] " << message;
            pending_.push_back(line.str());
        }
        can_log_.notify_one();
    }

    bool IsOpen() const {
        return stream_.is_open();
    }

private:
    void Stop() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopped_) {
                return;
            }
            stopped_ = true;
        }
        can_log_.notify_one();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    void WorkerLoop() {
        while (true) {
            std::deque<std::string> local;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                can_log_.wait(lock, [this]() {
                    return stopped_ || !pending_.empty();
                });

                if (pending_.empty() && stopped_) {
                    break;
                }

                // Move pending logs out quickly, then write without holding the
                // mutex. This keeps Log() cheap for audio threads.
                local.swap(pending_);
            }

            for (const auto& line : local) {
                if (stream_.is_open()) {
                    stream_ << line << '\n';
                }
                std::cerr << line << '\n';
            }
            if (stream_.is_open()) {
                stream_.flush();
            }
        }
    }

    mutable std::mutex mutex_;
    std::condition_variable can_log_;
    bool stopped_ = false;
    std::deque<std::string> pending_;
    std::ofstream stream_;
    std::thread worker_;
};

// Frame counters are cumulative. StatsLoop derives per-second rates and drift
// from these totals without touching the audio buffers.
struct RuntimeStats {
    std::atomic<uint64_t> input8_frames{0};
    std::atomic<uint64_t> input2_frames{0};
    std::atomic<uint64_t> mux_frames{0};
    std::atomic<uint64_t> uac2_output_frames{0};
    std::atomic<uint64_t> pending_output_frames{0};
    std::atomic<uint64_t> playback_input_frames{0};
    std::atomic<uint64_t> local_playback_frames{0};
    std::atomic<uint64_t> input8_errors{0};
    std::atomic<uint64_t> input2_errors{0};
    std::atomic<uint64_t> mux_errors{0};
    std::atomic<uint64_t> uac2_output_errors{0};
    std::atomic<uint64_t> playback_input_errors{0};
    std::atomic<uint64_t> local_playback_errors{0};
};

void PrintEndpoint(const char* name, const a1usbrecord::PcmEndpoint& endpoint) {
    std::cout << name << ": card=" << endpoint.card
              << ", device=" << endpoint.device
              << ", channels=" << endpoint.channels
              << ", rate=" << endpoint.rate
              << ", bits=" << endpoint.bits
              << ", period_size=" << endpoint.period_size
              << ", period_count=" << endpoint.period_count << '\n';
}

bool HasArg(int argc, char** argv, const std::string& expected) {
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == expected) {
            return true;
        }
    }
    return false;
}

int64_t Diff(uint64_t lhs, uint64_t rhs) {
    return static_cast<int64_t>(lhs) - static_cast<int64_t>(rhs);
}

void ApplyGain(std::vector<int16_t>* samples, float gain) {
    if (samples == nullptr || gain == 1.0f) {
        return;
    }

    for (int16_t& sample : *samples) {
        sample = static_cast<int16_t>(static_cast<float>(sample) * gain);
    }
}

void FillSilence(std::vector<int16_t>* samples) {
    if (samples == nullptr) {
        return;
    }

    std::fill(samples->begin(), samples->end(), 0);
}

bool EnsureDirectory(const char* path) {
    if (path == nullptr || path[0] == '\0') {
        return false;
    }

    if (mkdir(path, 0755) == 0) {
        return true;
    }

    if (errno != EEXIST) {
        return false;
    }

    struct stat info {};
    return stat(path, &info) == 0 && S_ISDIR(info.st_mode);
}

void CaptureLoop(const char* name,
                 a1usbrecord::PcmDevice* device,
                 const a1usbrecord::PcmEndpoint endpoint,
                 a1usbrecord::PcmChunkQueue* queue,
                 std::atomic<uint64_t>* frames_counter,
                 std::atomic<uint64_t>* error_counter,
                 FileLogger* logger,
                 std::atomic<bool>* running) {
    // One capture thread owns one ALSA capture PCM and pushes fixed-size chunks
    // into a queue consumed by the mux/write thread.
    while (running->load()) {
        a1usbrecord::PcmChunk chunk;
        chunk.frames = endpoint.period_size;
        chunk.channels = endpoint.channels;
        chunk.samples.resize(chunk.frames * chunk.channels);

        if (!device->ReadFrames(chunk.samples.data(), chunk.frames)) {
            error_counter->fetch_add(1);
            logger->Log(std::string("failed to read ") + name + ": " + device->LastError());
            running->store(false);
            queue->Stop();
            return;
        }

        frames_counter->fetch_add(chunk.frames);
        if (!queue->Push(std::move(chunk))) {
            return;
        }
    }
}

void AppendSamples(std::vector<int16_t>* target, const std::vector<int16_t>& source) {
    target->insert(target->end(), source.begin(), source.end());
}

bool WritePendingFrames(a1usbrecord::PcmDevice* output,
                        std::vector<int16_t>* pending,
                        std::size_t channels,
                        std::size_t frames_per_write,
                        RuntimeStats* stats) {
    // UAC2 accepts a smaller period than the capture sources. Keep a pending
    // buffer and write only complete output periods.
    while (pending->size() >= frames_per_write * channels) {
        if (!output->WriteFrames(pending->data(), frames_per_write)) {
            stats->uac2_output_errors.fetch_add(1);
            pending->clear();
            stats->pending_output_frames.store(0);
            return true;
        }

        stats->uac2_output_frames.fetch_add(frames_per_write);
        pending->erase(pending->begin(), pending->begin() + frames_per_write * channels);
        stats->pending_output_frames.store(pending->size() / channels);
    }

    return true;
}

void MuxWriteLoop(a1usbrecord::PcmChunkQueue* input8ch_queue,
                  a1usbrecord::PcmChunkQueue* input2ch_queue,
                  a1usbrecord::PcmDevice* output,
                  const a1usbrecord::PcmEndpoint output_endpoint,
                  const std::atomic<bool>* uac2_ready,
                  RuntimeStats* stats,
                  FileLogger* logger,
                  std::atomic<bool>* running) {
    a1usbrecord::AudioMuxer muxer;
    std::vector<int16_t> pending_output;

    // Combine card0 8ch and card1 2ch chunks into 10ch interleaved frames, then
    // write them to the UAC2 playback PCM exposed as the PC recording device.
    while (running->load()) {
        a1usbrecord::PcmChunk chunk8ch;
        a1usbrecord::PcmChunk chunk2ch;

        if (!input8ch_queue->Pop(&chunk8ch) || !input2ch_queue->Pop(&chunk2ch)) {
            running->store(false);
            break;
        }

        if (chunk8ch.frames != chunk2ch.frames) {
            stats->mux_errors.fetch_add(1);
            std::ostringstream message;
            message << "input frame count mismatch: " << chunk8ch.frames
                    << " vs " << chunk2ch.frames;
            logger->Log(message.str());
            running->store(false);
            break;
        }

        if (!uac2_ready->load()) {
            stats->pending_output_frames.store(0);
            continue;
        }

        std::vector<int16_t> merged(chunk8ch.frames * output_endpoint.channels);
        if (!muxer.Merge8Plus2To10(chunk8ch.samples.data(),
                                   chunk2ch.samples.data(),
                                   chunk8ch.frames,
                                   merged.data())) {
            stats->mux_errors.fetch_add(1);
            logger->Log("failed to mux audio buffers");
            running->store(false);
            break;
        }

        stats->mux_frames.fetch_add(chunk8ch.frames);
        AppendSamples(&pending_output, merged);
        stats->pending_output_frames.store(pending_output.size() / output_endpoint.channels);
        if (!WritePendingFrames(output,
                                &pending_output,
                                output_endpoint.channels,
                                output_endpoint.period_size,
                                stats)) {
            running->store(false);
            break;
        }
    }

    input8ch_queue->Stop();
    input2ch_queue->Stop();
}

void PlaybackCaptureLoop(a1usbrecord::PcmDevice* input,
                         const a1usbrecord::PcmEndpoint input_endpoint,
                         std::size_t buffer_frames,
                         unsigned int read_retry_ms,
                         float playback_gain,
                         a1usbrecord::PcmChunkQueue* playback_queue,
                         a1usbrecord::PcmChunkQueue* input8ch_queue,
                         a1usbrecord::PcmChunkQueue* input2ch_queue,
                         RuntimeStats* stats,
                         FileLogger* logger,
                         std::atomic<bool>* running) {
    // PC playback is the reverse UAC2 direction: read pcmC4D0c and push chunks
    // to a queue consumed by the local playback writer.
    while (running->load()) {
        a1usbrecord::PcmChunk chunk;
        chunk.frames = buffer_frames;
        chunk.channels = input_endpoint.channels;
        chunk.samples.resize(chunk.frames * chunk.channels);

        if (!input->ReadFrames(chunk.samples.data(), chunk.frames)) {
            stats->playback_input_errors.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(read_retry_ms));
            continue;
        }
        stats->playback_input_frames.fetch_add(chunk.frames);
        ApplyGain(&chunk.samples, playback_gain);

        if (!playback_queue->Push(std::move(chunk))) {
            break;
        }
    }

    playback_queue->Stop();
    input8ch_queue->Stop();
    input2ch_queue->Stop();
}

void PlaybackWriteLoop(a1usbrecord::PcmDevice* output,
                       const a1usbrecord::PcmEndpoint output_endpoint,
                       std::size_t chunk_frames,
                       a1usbrecord::PcmChunkQueue* playback_queue,
                       a1usbrecord::PcmChunkQueue* input8ch_queue,
                       a1usbrecord::PcmChunkQueue* input2ch_queue,
                       RuntimeStats* stats,
                       FileLogger* logger,
                       std::atomic<bool>* running) {
    a1usbrecord::PcmChunk chunk;
    chunk.frames = chunk_frames;
    chunk.channels = output_endpoint.channels;
    chunk.samples.resize(chunk.frames * chunk.channels);

    while (running->load()) {
        if (!playback_queue->TryPop(&chunk)) {
            chunk.frames = chunk_frames;
            chunk.channels = output_endpoint.channels;
            chunk.samples.resize(chunk.frames * chunk.channels);
            FillSilence(&chunk.samples);
        }

        if (!output->WriteFrames(chunk.samples.data(), chunk.frames)) {
            stats->local_playback_errors.fetch_add(1);
            logger->Log(std::string("failed to write localPlayback: ") + output->LastError());
            running->store(false);
            break;
        }
        stats->local_playback_frames.fetch_add(chunk.frames);
    }

    playback_queue->Stop();
    input8ch_queue->Stop();
    input2ch_queue->Stop();
}

void StatsLoop(const RuntimeStats* stats,
               const a1usbrecord::PcmChunkQueue* input8ch_queue,
               const a1usbrecord::PcmChunkQueue* input2ch_queue,
               const a1usbrecord::PcmChunkQueue* playback_queue,
               FileLogger* logger,
               unsigned int interval_seconds,
               std::condition_variable* stop_condition,
               std::mutex* stop_mutex,
               std::atomic<bool>* running) {
    uint64_t last_input8 = 0;
    uint64_t last_input2 = 0;
    uint64_t last_mux = 0;
    uint64_t last_uac2_output = 0;
    uint64_t last_playback_input = 0;
    uint64_t last_local_playback = 0;
    uint64_t last_input8_errors = 0;
    uint64_t last_input2_errors = 0;
    uint64_t last_mux_errors = 0;
    uint64_t last_uac2_output_errors = 0;
    uint64_t last_playback_input_errors = 0;
    uint64_t last_local_playback_errors = 0;

    // Log one summary per configured interval. Stable operation should keep
    // small queue depth and near-zero diff/lag counters over time.
    while (running->load()) {
        {
            std::unique_lock<std::mutex> lock(*stop_mutex);
            if (stop_condition->wait_for(lock,
                                         std::chrono::seconds(interval_seconds),
                                         [running]() { return !running->load(); })) {
                break;
            }
        }

        const uint64_t input8 = stats->input8_frames.load();
        const uint64_t input2 = stats->input2_frames.load();
        const uint64_t mux = stats->mux_frames.load();
        const uint64_t uac2_output = stats->uac2_output_frames.load();
        const uint64_t pending_output = stats->pending_output_frames.load();
        const uint64_t playback_input = stats->playback_input_frames.load();
        const uint64_t local_playback = stats->local_playback_frames.load();
        const uint64_t input8_errors = stats->input8_errors.load();
        const uint64_t input2_errors = stats->input2_errors.load();
        const uint64_t mux_errors = stats->mux_errors.load();
        const uint64_t uac2_output_errors = stats->uac2_output_errors.load();
        const uint64_t playback_input_errors = stats->playback_input_errors.load();
        const uint64_t local_playback_errors = stats->local_playback_errors.load();

        const uint64_t input8_interval = input8 - last_input8;
        const uint64_t input2_interval = input2 - last_input2;
        const uint64_t mux_interval = mux - last_mux;
        const uint64_t uac2_output_interval = uac2_output - last_uac2_output;
        const uint64_t playback_input_interval = playback_input - last_playback_input;
        const uint64_t local_playback_interval = local_playback - last_local_playback;

        std::ostringstream total_message;
        total_message << "stats_total"
                      << " interval_seconds=" << interval_seconds
                      << " input8_total=" << input8
                      << " input2_total=" << input2
                      << " capture_diff_frames=" << Diff(input8, input2)
                      << " mux_total=" << mux
                      << " uac2_out_total=" << uac2_output
                      << " uac2_lag_frames=" << Diff(mux, uac2_output)
                      << " pending_out_frames=" << pending_output
                      << " playback_in_total=" << playback_input
                      << " local_playback_total=" << local_playback
                      << " playback_diff_frames=" << Diff(playback_input, local_playback)
                      << " q8=" << input8ch_queue->Size() << "/" << input8ch_queue->Capacity()
                      << " q2=" << input2ch_queue->Size() << "/" << input2ch_queue->Capacity()
                      << " qplay=" << playback_queue->Size() << "/" << playback_queue->Capacity()
                      << " errors="
                      << "i8:" << input8_errors
                      << ",i2:" << input2_errors
                      << ",mux:" << mux_errors
                      << ",uac2out:" << uac2_output_errors
                      << ",playin:" << playback_input_errors
                      << ",playout:" << local_playback_errors;
        logger->Log(total_message.str());

        std::ostringstream interval_message;
        interval_message << "stats_interval"
                         << " interval_seconds=" << interval_seconds
                         << " input8_interval_frames=" << input8_interval
                         << " input2_interval_frames=" << input2_interval
                         << " capture_interval_diff_frames=" << Diff(input8_interval, input2_interval)
                         << " mux_interval_frames=" << mux_interval
                         << " uac2_out_interval_frames=" << uac2_output_interval
                         << " uac2_interval_lag_frames=" << Diff(mux_interval, uac2_output_interval)
                         << " playback_in_interval_frames=" << playback_input_interval
                         << " local_playback_interval_frames=" << local_playback_interval
                         << " playback_interval_diff_frames="
                         << Diff(playback_input_interval, local_playback_interval)
                         << " interval_errors="
                         << "i8:" << (input8_errors - last_input8_errors)
                         << ",i2:" << (input2_errors - last_input2_errors)
                         << ",mux:" << (mux_errors - last_mux_errors)
                         << ",uac2out:" << (uac2_output_errors - last_uac2_output_errors)
                         << ",playin:" << (playback_input_errors - last_playback_input_errors)
                         << ",playout:" << (local_playback_errors - last_local_playback_errors);
        logger->Log(interval_message.str());

        last_input8 = input8;
        last_input2 = input2;
        last_mux = mux;
        last_uac2_output = uac2_output;
        last_playback_input = playback_input;
        last_local_playback = local_playback;
        last_input8_errors = input8_errors;
        last_input2_errors = input2_errors;
        last_mux_errors = mux_errors;
        last_uac2_output_errors = uac2_output_errors;
        last_playback_input_errors = playback_input_errors;
        last_local_playback_errors = local_playback_errors;
    }
}

}  // namespace

int main(int argc, char** argv) {
    a1usbrecord::AppConfig config;
    config.dry_run = HasArg(argc, argv, "--dry-run");

    std::cout << "av_virtual 8+2 channel UAC2 bridge\n";
    PrintEndpoint("input8ch", config.input8ch);
    PrintEndpoint("input2ch", config.input2ch);
    PrintEndpoint("uac2Output", config.uac2Output);
    PrintEndpoint("uac2Input", config.uac2Input);
    PrintEndpoint("localPlayback", config.localPlayback);
    std::cout << "logDir: " << config.logDir << '\n';
    std::cout << "logPath: " << config.logPath << '\n';
    std::cout << "statsLogIntervalSeconds: " << config.statsLogIntervalSeconds << '\n';
    std::cout << "uacOpenDelayMs: " << config.uacOpenDelayMs << '\n';
    std::cout << "playbackQueueChunkFrames: " << config.playbackQueueChunkFrames << '\n';
    std::cout << "uacInputReadRetryMs: " << config.uacInputReadRetryMs << '\n';
    std::cout << "uacPlaybackGain: " << config.uacPlaybackGain << '\n';

    if (config.dry_run) {
        std::cout << "dry-run mode. Run without --dry-run to open PCM devices.\n";
        return 0;
    }

    a1usbrecord::PcmDevice input8ch(config.input8ch, a1usbrecord::PcmDirection::Capture);
    a1usbrecord::PcmDevice input2ch(config.input2ch, a1usbrecord::PcmDirection::Capture);
    a1usbrecord::PcmDevice uac2Output(config.uac2Output, a1usbrecord::PcmDirection::Playback);
    a1usbrecord::PcmDevice uac2Input(config.uac2Input, a1usbrecord::PcmDirection::Capture);
    a1usbrecord::PcmDevice localPlayback(config.localPlayback, a1usbrecord::PcmDirection::Playback);

    if (!input8ch.Open()) {
        std::cerr << "failed to open input8ch: " << input8ch.LastError() << '\n';
        return 1;
    }
    if (!input2ch.Open()) {
        std::cerr << "failed to open input2ch: " << input2ch.LastError() << '\n';
        return 1;
    }
    if (!localPlayback.Open()) {
        std::cerr << "failed to open localPlayback: " << localPlayback.LastError() << '\n';
        return 1;
    }

    if (!EnsureDirectory(config.logDir)) {
        std::cerr << "failed to create log dir: " << config.logDir << '\n';
    }

    FileLogger logger(config.logPath);
    if (!logger.IsOpen()) {
        std::cerr << "failed to open log file: " << config.logPath << '\n';
    }
    logger.Log("av_virtual started");

    constexpr std::size_t kQueueCapacity = 8;
    std::atomic<bool> running{true};
    std::atomic<bool> uac2_ready{false};
    RuntimeStats stats;
    std::mutex stop_mutex;
    std::condition_variable stop_condition;
    a1usbrecord::PcmChunkQueue input8ch_queue(kQueueCapacity);
    a1usbrecord::PcmChunkQueue input2ch_queue(kQueueCapacity);
    a1usbrecord::PcmChunkQueue playback_queue(kQueueCapacity);

    std::thread input8ch_thread(CaptureLoop,
                                "input8ch",
                                &input8ch,
                                config.input8ch,
                                &input8ch_queue,
                                &stats.input8_frames,
                                &stats.input8_errors,
                                &logger,
                                &running);
    std::thread input2ch_thread(CaptureLoop,
                                "input2ch",
                                &input2ch,
                                config.input2ch,
                                &input2ch_queue,
                                &stats.input2_frames,
                                &stats.input2_errors,
                                &logger,
                                &running);
    std::thread mux_write_thread(MuxWriteLoop,
                                 &input8ch_queue,
                                 &input2ch_queue,
                                 &uac2Output,
                                 config.uac2Output,
                                 &uac2_ready,
                                 &stats,
                                 &logger,
                                 &running);
    std::thread playback_write_thread(PlaybackWriteLoop,
                                      &localPlayback,
                                      config.localPlayback,
                                      config.playbackQueueChunkFrames,
                                      &playback_queue,
                                      &input8ch_queue,
                                      &input2ch_queue,
                                      &stats,
                                      &logger,
                                      &running);
    std::thread stats_thread(StatsLoop,
                             &stats,
                             &input8ch_queue,
                             &input2ch_queue,
                             &playback_queue,
                             &logger,
                             config.statsLogIntervalSeconds,
                             &stop_condition,
                             &stop_mutex,
                             &running);

    logger.Log("delay opening card4 UAC2 devices and drop pre-open data");
    std::this_thread::sleep_for(std::chrono::milliseconds(config.uacOpenDelayMs));

    if (!uac2Output.Open()) {
        logger.Log(std::string("failed to open uac2Output: ") + uac2Output.LastError());
        running.store(false);
    }
    if (running.load() && !uac2Input.Open()) {
        logger.Log(std::string("failed to open uac2Input: ") + uac2Input.LastError());
        running.store(false);
    }

    std::thread playback_capture_thread;
    if (running.load()) {
        uac2_ready.store(true);
        logger.Log("card4 UAC2 devices opened");
        playback_capture_thread = std::thread(PlaybackCaptureLoop,
                                              &uac2Input,
                                              config.uac2Input,
                                              config.playbackQueueChunkFrames,
                                              config.uacInputReadRetryMs,
                                              config.uacPlaybackGain,
                                              &playback_queue,
                                              &input8ch_queue,
                                              &input2ch_queue,
                                              &stats,
                                              &logger,
                                              &running);
    }

    if (!running.load()) {
        input8ch_queue.Stop();
        input2ch_queue.Stop();
        playback_queue.Stop();
    }

    mux_write_thread.join();
    running.store(false);
    stop_condition.notify_all();
    input8ch_queue.Stop();
    input2ch_queue.Stop();
    playback_queue.Stop();
    input8ch_thread.join();
    input2ch_thread.join();
    if (playback_capture_thread.joinable()) {
        playback_capture_thread.join();
    }
    playback_write_thread.join();
    stats_thread.join();
    logger.Log("av_virtual stopped");

    return 1;
}
