#pragma once
#include "wm/transform/subband.h"

namespace wm {

// Iterate over all full 8×8 blocks in a subband
template <typename Fn>
void for_each_block_8x8(const SubbandView& band, Fn&& fn) {
    for (uint32_t by = 0; by + 8 <= band.height; by += 8) {
        for (uint32_t bx = 0; bx + 8 <= band.width; bx += 8) {
            float* block =
                band.data + by * band.stride + bx;
            fn(block, band.stride);
        }
    }
}

} // namespace wm
