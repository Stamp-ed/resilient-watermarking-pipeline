#pragma once
#include <cstdint>
#include "wm/image.h"

namespace wm {

// Embed a payload into an image (in-place luminance assumed)
bool embed_image(
    Image& img,
    const int8_t* payload_bits, // length = payload_len
    uint32_t payload_len,
    uint64_t key,
    float alpha
);

}
