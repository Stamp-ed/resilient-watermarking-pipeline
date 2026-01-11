#pragma once
#include <cstdint>

namespace wm {

void normalize_luminance_blocks(
    float* Y,
    uint32_t width,
    uint32_t height,
    uint32_t block_size = 32
);

}
