#pragma once
#include <cstdint>

namespace wm {

// Generate a deterministic permutation of [0..total_blocks-1]
void generate_block_permutation(
    uint64_t key,
    uint32_t* perm,
    uint32_t total_blocks
);

}
