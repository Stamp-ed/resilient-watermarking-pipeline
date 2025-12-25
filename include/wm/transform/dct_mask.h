#pragma once
#include <cstdint>

namespace wm {

struct DCTIndex {
    uint8_t u;
    uint8_t v;
};

// Fixed mid-frequency mask (v1)
constexpr uint32_t DCT_MASK_SIZE = 7;

extern const DCTIndex DCT_MID_FREQ_MASK[DCT_MASK_SIZE];

} // namespace wm
