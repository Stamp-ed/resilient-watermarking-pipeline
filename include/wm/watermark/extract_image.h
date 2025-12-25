#pragma once
#include <cstdint>
#include "wm/image.h"

namespace wm {

// Extract payload and per-bit confidence from image
bool extract_image(
    Image& img,
    int8_t* bits_out,         // length = payload_len
    float* confidence_out,    // length = payload_len
    uint32_t payload_len,
    uint64_t key
);

}
