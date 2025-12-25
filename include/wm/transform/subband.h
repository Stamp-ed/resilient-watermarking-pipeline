#pragma once
#include <cstdint>

namespace wm {

struct SubbandView {
    float* data;
    uint32_t width;
    uint32_t height;
    uint32_t stride; // full image width
};

// 2-level Haar subbands
SubbandView LL2(float* data, uint32_t W, uint32_t H);
SubbandView HL2(float* data, uint32_t W, uint32_t H);
SubbandView LH2(float* data, uint32_t W, uint32_t H);
SubbandView HH2(float* data, uint32_t W, uint32_t H);

} // namespace wm
