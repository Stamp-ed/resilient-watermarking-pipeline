// include/wm/v2/qimd.h
#pragma once
#include <cstdint>
#include <cmath>

namespace wm::v2 {

// Embed bit into one coefficient
inline float qim_embed(float c, int bit, float step) {
    float q = 2.0f * step;
    float base = std::floor(c / q) * q;
    return base + (bit > 0 ? step : -step);
}

// Extract bit from one coefficient
inline int qim_extract(float c, float step) {
    float q = 2.0f * step;
    float r = std::fmod(std::fabs(c), q);
    return (r > step) ? +1 : -1;
}

}
