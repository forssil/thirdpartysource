#pragma once

#include <cstddef>
#include <cstdint>

namespace a1usbrecord {

class AudioMuxer {
public:
    bool Merge8Plus2To10(const int16_t* input8ch,
                         const int16_t* input2ch,
                         std::size_t frames,
                         int16_t* output10ch) const;
};

}  // namespace a1usbrecord
