#pragma once
#include <cstdint>

namespace wm {

// Generate PN chip in { -1, +1 }
int8_t pn_chip(uint64_t key,
               uint32_t bit_index,
               uint32_t block_index,
               uint32_t chip_index);

} // namespace wm
