#include "wm/preprocess/normalize.h"
#include <cmath>
#include <algorithm>

namespace wm {

void normalize_luminance_blocks(
    float* Y,
    uint32_t width,
    uint32_t height,
    uint32_t block_size
) {
    const float eps = 1e-3f;

    for (uint32_t by = 0; by < height; by += block_size) {
        for (uint32_t bx = 0; bx < width; bx += block_size) {

            // 1. Compute local mean
            float sum = 0.0f;
            float sum2 = 0.0f;
            uint32_t count = 0;

            for (uint32_t y = 0; y < block_size; ++y) {
                for (uint32_t x = 0; x < block_size; ++x) {
                    uint32_t iy = by + y;
                    uint32_t ix = bx + x;
                    if (iy >= height || ix >= width) continue;

                    float v = Y[iy * width + ix];
                    sum += v;
                    sum2 += v * v;
                    count++;
                }
            }

            if (count == 0) continue;

            float mean = sum / count;
            float var  = sum2 / count - mean * mean;
            float std  = std::sqrt(std::max(var, 0.0f));

            // 2. Normalize block
            for (uint32_t y = 0; y < block_size; ++y) {
                for (uint32_t x = 0; x < block_size; ++x) {
                    uint32_t iy = by + y;
                    uint32_t ix = bx + x;
                    if (iy >= height || ix >= width) continue;

                    float& v = Y[iy * width + ix];
                    v = (v - mean) / (std + eps);
                }
            }
        }
    }
}

}
