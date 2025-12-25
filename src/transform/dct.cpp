#include "wm/transform/dct.h"
#include <cmath>

namespace wm {

static constexpr float PI = 3.14159265358979323846f;

static inline float alpha(int k) {
    return (k == 0) ? (1.0f / std::sqrt(8.0f))
                    : std::sqrt(2.0f / 8.0f);
}

// -------------------------
// Forward DCT
// -------------------------
void    dct8x8(const float* input, float* output) {
    for (int u = 0; u < 8; ++u) {
        for (int v = 0; v < 8; ++v) {
            float sum = 0.0f;

            for (int x = 0; x < 8; ++x) {
                for (int y = 0; y < 8; ++y) {
                    float pixel = input[x * 8 + y];
                    float cx = std::cos((2.0f * x + 1.0f) * u * PI / 16.0f);
                    float cy = std::cos((2.0f * y + 1.0f) * v * PI / 16.0f);
                    sum += pixel * cx * cy;
                }
            }

            output[u * 8 + v] = alpha(u) * alpha(v) * sum;
        }
    }
}

// -------------------------
// Inverse DCT
// -------------------------
void idct8x8(const float* input, float* output) {
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            float sum = 0.0f;

            for (int u = 0; u < 8; ++u) {
                for (int v = 0; v < 8; ++v) {
                    float coeff = input[u * 8 + v];
                    float cx = std::cos((2.0f * x + 1.0f) * u * PI / 16.0f);
                    float cy = std::cos((2.0f * y + 1.0f) * v * PI / 16.0f);
                    sum += alpha(u) * alpha(v) * coeff * cx * cy;
                }
            }

            output[x * 8 + y] = sum;
        }
    }
}

} // namespace wm
