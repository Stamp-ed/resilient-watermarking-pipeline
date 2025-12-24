#pragma once
#include <cstdint>

namespace wm {

// Embed a single watermark bit into one 8×8 block (in-place)
void embed_bit_block(
    float* spatial_block,   // pointer to top-left of 8×8 block
    uint32_t stride,         // image stride
    int8_t bit,              // +1 or -1
    uint64_t key,            // global watermark key
    uint32_t bit_index,      // which watermark bit
    uint32_t block_index,    // block id (spatial index)
    float alpha              // embedding strength
);

} // namespace wm
