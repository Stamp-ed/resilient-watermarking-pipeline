#include "wm/transform/dwt.h"
#include <cmath>
#include <vector>
#include <cassert>

namespace wm {

static constexpr float INV_SQRT2 = 0.7071067811865475f;

// --------------------------------
// 1D Haar forward 
// --------------------------------
static void haar_1d(float* data, uint32_t n) {
    assert(n % 2 == 0);

    std::vector<float> temp(n);

    uint32_t half = n / 2;
    for (uint32_t i = 0; i < half; ++i) {
        float a = data[2 * i];
        float b = data[2 * i + 1];
        temp[i]       = (a + b) * INV_SQRT2;
        temp[i + half]= (a - b) * INV_SQRT2;
    }

    for (uint32_t i = 0; i < n; ++i)
        data[i] = temp[i];
}

// --------------------------------
// 1D Haar inverse
// --------------------------------
static void ihaar_1d(float* data, uint32_t n) {
    assert(n % 2 == 0);

    std::vector<float> temp(n);
    uint32_t half = n / 2;

    for (uint32_t i = 0; i < half; ++i) {
        float a = data[i];
        float d = data[i + half];
        temp[2 * i]     = (a + d) * INV_SQRT2;
        temp[2 * i + 1]= (a - d) * INV_SQRT2;
    }

    for (uint32_t i = 0; i < n; ++i)
        data[i] = temp[i];
}

// --------------------------------
// 2D Haar DWT (2 levels)
// --------------------------------
void dwt2_haar(float* data, uint32_t width, uint32_t height) {
    assert(width % 4 == 0 && height % 4 == 0);

    uint32_t w = width;
    uint32_t h = height;

    for (int level = 0; level < 2; ++level) {

        // Rows
        for (uint32_t y = 0; y < h; ++y)
            haar_1d(&data[y * width], w);

        // Columns
        std::vector<float> col(h);
        for (uint32_t x = 0; x < w; ++x) {
            for (uint32_t y = 0; y < h; ++y)
                col[y] = data[y * width + x];

            haar_1d(col.data(), h);

            for (uint32_t y = 0; y < h; ++y)
                data[y * width + x] = col[y];
        }

        w /= 2;
        h /= 2;
    }
}

// --------------------------------
// 2D Haar inverse (2 levels)
// --------------------------------
void idwt2_haar(float* data, uint32_t width, uint32_t height) {
    assert(width % 4 == 0 && height % 4 == 0);

    // Start from smallest LL band (after 2 levels)
    uint32_t w = width / 4;
    uint32_t h = height / 4;

    for (int level = 0; level < 2; ++level) {

        // Inverse columns
        std::vector<float> col(h * 2);
        for (uint32_t x = 0; x < w * 2; ++x) {
            for (uint32_t y = 0; y < h * 2; ++y)
                col[y] = data[y * width + x];

            ihaar_1d(col.data(), h * 2);

            for (uint32_t y = 0; y < h * 2; ++y)
                data[y * width + x] = col[y];
        }

        // Inverse rows
        for (uint32_t y = 0; y < h * 2; ++y)
            ihaar_1d(&data[y * width], w * 2);

        // Expand for next level
        w *= 2;
        h *= 2;
    }
}


} // namespace wm
