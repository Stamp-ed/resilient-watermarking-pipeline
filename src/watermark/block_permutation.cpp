#include "wm/watermark/block_permutation.h"

namespace wm {

// SplitMix64 (reuse same primitive)
static inline uint64_t splitmix64(uint64_t& x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

void generate_block_permutation(
    uint64_t key,
    uint32_t* perm,
    uint32_t total_blocks
) {
    // Initialize identity permutation
    for (uint32_t i = 0; i < total_blocks; ++i)
        perm[i] = i;

    uint64_t seed = key ^ 0xA5A5A5A5A5A5A5A5ULL;

    // Fisher–Yates shuffle
    for (uint32_t i = total_blocks - 1; i > 0; --i) {
        uint64_t r = splitmix64(seed);
        uint32_t j = r % (i + 1);

        uint32_t tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }
}

}
