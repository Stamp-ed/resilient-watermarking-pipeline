#pragma once
#include "wm/transform/subband.h"

namespace wm {

// Apply DCT + IDCT to all 8×8 blocks in subband
void dct_roundtrip_subband(const SubbandView& band);

} // namespace wm
