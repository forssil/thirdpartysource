#include "a1usbrecord/audio_muxer.h"

#include "a1usbrecord/app_config.h"

namespace a1usbrecord {

bool AudioMuxer::Merge8Plus2To10(const int16_t* input8ch,
                                 const int16_t* input2ch,
                                 std::size_t frames,
                                 int16_t* output10ch) const {
    if (input8ch == nullptr || input2ch == nullptr || output10ch == nullptr) {
        return false;
    }

    for (std::size_t frame = 0; frame < frames; ++frame) {
        const std::size_t in8_base = frame * kInput8Channels;
        const std::size_t in2_base = frame * kInput2Channels;
        const std::size_t out_base = frame * kMergedChannels;

        for (std::size_t channel = 0; channel < kInput8Channels; ++channel) {
            output10ch[out_base + channel] = input8ch[in8_base + channel];
        }

        output10ch[out_base + 8] = input2ch[in2_base];
        output10ch[out_base + 9] = input2ch[in2_base + 1];
    }

    return true;
}

}  // namespace a1usbrecord
