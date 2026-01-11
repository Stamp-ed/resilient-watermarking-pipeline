// include/wm/v2/patch.h
#pragma once
#include <vector>
#include "wm/image.h"
#include "region.h"

namespace wm::v2 {

// Extracts and resamples region → 48×48 patch
bool extract_patch(
    const wm::Image& img,
    const Region& region,
    float* out_patch   // size = PATCH_SIZE * PATCH_SIZE
);

// Writes patch back into image (for embedding)
bool write_patch(
    wm::Image& img,
    const Region& region,
    const float* patch
);

}