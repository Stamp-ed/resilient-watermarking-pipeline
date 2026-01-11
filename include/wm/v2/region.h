// include/wm/v2/region.h
#pragma once
#include <cstdint>

namespace wm::v2 {

struct Region {
    float cx;      // center x (image space)
    float cy;      // center y
    float scale;   // region scale (1.0 = PATCH_SIZE)
};

}