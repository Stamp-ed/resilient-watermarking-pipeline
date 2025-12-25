#pragma once
#include <cstdint>

namespace wm {

// Extract a single watermark bit from one 8×8 block
// Returns +1 or -1
int8_t extract_bit_block(
    const float* spatial_block, // pointer to top-left of 8×8 block
    uint32_t stride,             // image stride
    uint64_t key,                // global watermark key
    uint32_t bit_index,          // which watermark bit
    uint32_t block_index         // block id (spatial index)
);

} // namespace wm
