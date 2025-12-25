#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include "wm/transform/dwt.h"

using namespace wm;

static bool nearly_equal(float a, float b, float eps = 1e-3f) {
    return std::fabs(a - b) < eps;
}

// ----------------------------
// Test 1: Round-trip
// ----------------------------
void test_round_trip() {
    const uint32_t W = 32, H = 32;
    std::vector<float> img(W * H);

    for (uint32_t i = 0; i < W * H; ++i)
        img[i] = static_cast<float>(i % 17);

    std::vector<float> original = img;

    dwt2_haar(img.data(), W, H);
    idwt2_haar(img.data(), W, H);

    for (uint32_t i = 0; i < W * H; ++i)
        assert(nearly_equal(img[i], original[i]));

    printf("[PASS] DWT round-trip\n");
}

// ----------------------------
// Test 2: Constant image
// ----------------------------
void test_constant_image() {
    const uint32_t W = 32, H = 32;
    std::vector<float> img(W * H, 10.0f);

    dwt2_haar(img.data(), W, H);

    // Detail coefficients should be near zero
    for (uint32_t y = H / 2; y < H; ++y)
        for (uint32_t x = 0; x < W; ++x)
            assert(std::fabs(img[y * W + x]) < 1e-3f);

    printf("[PASS] DWT constant image\n");
}

// ----------------------------
// Main
// ----------------------------
int main() {
    test_round_trip();
    test_constant_image();

    printf("All DWT tests passed.\n");
    return 0;
}
