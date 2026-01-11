// src/wm/v2/embed_patch.cpp
#include "wm/v2/qimd.h"
#include "wm/transform/dwt.h"
#include "wm/transform/dct.h"
#include "wm/v2/constants.h"

namespace wm::v2 {

bool embed_patch(
    float* patch,           // 48×48
    const int8_t* bits,     // payload
    uint32_t bit_count
) {
    // 1. 3-level DWT
    dwt2_haar(patch, PATCH_SIZE, PATCH_SIZE);

    // 2. Select HL2 / LH2 / HL1 / LH1
    // 3. DCT blocks
    // 4. QIM embed bits (round-robin across coeffs)

    return true;
}

}
