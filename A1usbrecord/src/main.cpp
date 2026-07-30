#include <cstdint>
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "a1usbrecord/app_config.h"
#include "a1usbrecord/audio_muxer.h"
#include "a1usbrecord/pcm_chunk_queue.h"
#include "a1usbrecord/pcm_device.h"

namespace {

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

void CaptureLoop(const char* name,
                 a1usbrecord::PcmDevice* device,
                 const a1usbrecord::PcmEndpoint endpoint,
                 a1usbrecord::PcmChunkQueue* queue,
                 std::atomic<bool>* running) {
    while (running->load()) {
        a1usbrecord::PcmChunk chunk;
        chunk.frames = endpoint.period_size;
        chunk.channels = endpoint.channels;
        chunk.samples.resize(chunk.frames * chunk.channels);

        if (!device->ReadFrames(chunk.samples.data(), chunk.frames)) {
            std::cerr << "failed to read " << name << ": " << device->LastError() << '\n';
            running->store(false);
            queue->Stop();
            return;
        }

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
                        std::size_t frames_per_write) {
    while (pending->size() >= frames_per_write * channels) {
        if (!output->WriteFrames(pending->data(), frames_per_write)) {
            std::cerr << "failed to write uac2Output: " << output->LastError() << '\n';
            return false;
        }

        pending->erase(pending->begin(), pending->begin() + frames_per_write * channels);
    }

    return true;
}

void MuxWriteLoop(a1usbrecord::PcmChunkQueue* input8ch_queue,
                  a1usbrecord::PcmChunkQueue* input2ch_queue,
                  a1usbrecord::PcmDevice* output,
                  const a1usbrecord::PcmEndpoint output_endpoint,
                  std::atomic<bool>* running) {
    a1usbrecord::AudioMuxer muxer;
    std::vector<int16_t> pending_output;

    while (running->load()) {
        a1usbrecord::PcmChunk chunk8ch;
        a1usbrecord::PcmChunk chunk2ch;

        if (!input8ch_queue->Pop(&chunk8ch) || !input2ch_queue->Pop(&chunk2ch)) {
            running->store(false);
            break;
        }

        if (chunk8ch.frames != chunk2ch.frames) {
            std::cerr << "input frame count mismatch: " << chunk8ch.frames
                      << " vs " << chunk2ch.frames << '\n';
            running->store(false);
            break;
        }

        std::vector<int16_t> merged(chunk8ch.frames * output_endpoint.channels);
        if (!muxer.Merge8Plus2To10(chunk8ch.samples.data(),
                                   chunk2ch.samples.data(),
                                   chunk8ch.frames,
                                   merged.data())) {
            std::cerr << "failed to mux audio buffers\n";
            running->store(false);
            break;
        }

        AppendSamples(&pending_output, merged);
        if (!WritePendingFrames(output,
                                &pending_output,
                                output_endpoint.channels,
                                output_endpoint.period_size)) {
            running->store(false);
            break;
        }
    }

    input8ch_queue->Stop();
    input2ch_queue->Stop();
}

}  // namespace

int main(int argc, char** argv) {
    a1usbrecord::AppConfig config;
    config.dry_run = HasArg(argc, argv, "--dry-run");

    std::cout << "av_virtual 8+2 channel UAC2 bridge\n";
    PrintEndpoint("input8ch", config.input8ch);
    PrintEndpoint("input2ch", config.input2ch);
    PrintEndpoint("uac2Output", config.uac2Output);

    if (config.dry_run) {
        std::cout << "dry-run mode. Run without --dry-run to open PCM devices.\n";
        return 0;
    }

    a1usbrecord::PcmDevice input8ch(config.input8ch, a1usbrecord::PcmDirection::Capture);
    a1usbrecord::PcmDevice input2ch(config.input2ch, a1usbrecord::PcmDirection::Capture);
    a1usbrecord::PcmDevice uac2Output(config.uac2Output, a1usbrecord::PcmDirection::Playback);

    if (!input8ch.Open()) {
        std::cerr << "failed to open input8ch: " << input8ch.LastError() << '\n';
        return 1;
    }
    if (!input2ch.Open()) {
        std::cerr << "failed to open input2ch: " << input2ch.LastError() << '\n';
        return 1;
    }
    if (!uac2Output.Open()) {
        std::cerr << "failed to open uac2Output: " << uac2Output.LastError() << '\n';
        return 1;
    }

    constexpr std::size_t kQueueCapacity = 8;
    std::atomic<bool> running{true};
    a1usbrecord::PcmChunkQueue input8ch_queue(kQueueCapacity);
    a1usbrecord::PcmChunkQueue input2ch_queue(kQueueCapacity);

    std::thread input8ch_thread(CaptureLoop,
                                "input8ch",
                                &input8ch,
                                config.input8ch,
                                &input8ch_queue,
                                &running);
    std::thread input2ch_thread(CaptureLoop,
                                "input2ch",
                                &input2ch,
                                config.input2ch,
                                &input2ch_queue,
                                &running);
    std::thread mux_write_thread(MuxWriteLoop,
                                 &input8ch_queue,
                                 &input2ch_queue,
                                 &uac2Output,
                                 config.uac2Output,
                                 &running);

    mux_write_thread.join();
    running.store(false);
    input8ch_queue.Stop();
    input2ch_queue.Stop();
    input8ch_thread.join();
    input2ch_thread.join();

    return 1;
}
