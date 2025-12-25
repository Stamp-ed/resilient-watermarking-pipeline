#include "wm/watermark/pn.h"

namespace wm {

// SplitMix64
static inline uint64_t splitmix64(uint64_t& x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

int8_t pn_chip(uint64_t key,
               uint32_t bit_index,
               uint32_t block_index,
               uint32_t chip_index)
{
    uint64_t seed = key;
    seed ^= uint64_t(bit_index)  * 0x100000001b3ULL;
    seed ^= uint64_t(block_index) * 0xC6A4A7935BD1E995ULL;
    seed ^= uint64_t(chip_index) * 0x9E3779B97F4A7C15ULL;

    uint64_t x = seed;
    uint64_t r = splitmix64(x);

    return (r & 1ULL) ? +1 : -1;
}

} // namespace wm
#include <vector>