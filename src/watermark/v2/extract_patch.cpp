// src/wm/v2/extract_patch.cpp
#include "wm/v2/qimd.h"
#include "wm/transform/dwt.h"
#include "wm/transform/dct.h"

namespace wm::v2 {

bool extract_patch(
    const float* patch,
    int8_t* out_bits,
    float* confidence,
    uint32_t bit_count
) {
    // Mirror embed
    // Majority vote per bit
    return true;
}

}
