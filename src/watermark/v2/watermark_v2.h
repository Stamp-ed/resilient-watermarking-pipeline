// include/wm/v2/watermark_v2.h
#pragma once
#include "wm/image.h"
#include "region.h"

namespace wm::v2 {

bool embed_v2(
    wm::Image& img,
    const Region* regions,
    uint32_t region_count,
    const int8_t* payload,
    uint32_t payload_bits
);

bool extract_v2(
    const wm::Image& img,
    const Region* regions,
    uint32_t region_count,
    int8_t* out_bits,
    float* confidence
);

}
