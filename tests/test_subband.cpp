#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include "wm/transform/dwt.h"
#include "wm/transform/subband.h"

using namespace wm;

static bool nearly_equal(float a, float b, float eps = 1e-3f) {
    return std::fabs(a - b) < eps;
}

// ----------------------------
// Test 1: Dimensions
// ----------------------------
void test_subband_dimensions() {
    const uint32_t W = 32, H = 32;
    std::vector<float> img(W * H, 1.0f);

    dwt2_haar(img.data(), W, H);

    auto ll = LL2(img.data(), W, H);
    auto hl = HL2(img.data(), W, H);
    auto lh = LH2(img.data(), W, H);
    auto hh = HH2(img.data(), W, H);

    assert(ll.width == 8 && ll.height == 8);
    assert(hl.width == 8 && hl.height == 8);
    assert(lh.width == 8 && lh.height == 8);
    assert(hh.width == 8 && hh.height == 8);

    printf("[PASS] Subband dimensions\n");
}

// ----------------------------
// Test 2: Constant image
// ----------------------------
void test_constant_image_subbands() {
    const uint32_t W = 32, H = 32;
    std::vector<float> img(W * H, 10.0f);

    dwt2_haar(img.data(), W, H);

    auto hl = HL2(img.data(), W, H);
    auto lh = LH2(img.data(), W, H);
    auto hh = HH2(img.data(), W, H);

    for (uint32_t y = 0; y < hl.height; ++y)
        for (uint32_t x = 0; x < hl.width; ++x) {
            assert(std::fabs(hl.data[y * hl.stride + x]) < 1e-3f);
            assert(std::fabs(lh.data[y * lh.stride + x]) < 1e-3f);
            assert(std::fabs(hh.data[y * hh.stride + x]) < 1e-3f);
        }

    printf("[PASS] Constant image subbands\n");
}

void test_vertical_edge() {
    const uint32_t W = 32, H = 32;
    std::vector<float> img(W * H, 0.0f);

    for (uint32_t x = W / 2; x < W; ++x)
        for (uint32_t y = 0; y < H; ++y)
            img[y * W + x] = 100.0f;

    dwt2_haar(img.data(), W, H);

    auto hl = HL2(img.data(), W, H);
    auto lh = LH2(img.data(), W, H);

    float hl_energy = 0.0f;
    float lh_energy = 0.0f;

    for (uint32_t y = 0; y < hl.height; ++y)
        for (uint32_t x = 0; x < hl.width; ++x) {
            hl_energy += std::fabs(hl.data[y * hl.stride + x]);
            lh_energy += std::fabs(lh.data[y * lh.stride + x]);
        }

    assert(hl_energy > lh_energy);

    printf("[PASS] Vertical edge → HL energy\n");
}

void test_horizontal_edge() {
    const uint32_t W = 32, H = 32;
    std::vector<float> img(W * H, 0.0f);

    for (uint32_t y = H / 2; y < H; ++y)
        for (uint32_t x = 0; x < W; ++x)
            img[y * W + x] = 100.0f;

    dwt2_haar(img.data(), W, H);

    auto hl = HL2(img.data(), W, H);
    auto lh = LH2(img.data(), W, H);

    float hl_energy = 0.0f;
    float lh_energy = 0.0f;

    for (uint32_t y = 0; y < hl.height; ++y)
        for (uint32_t x = 0; x < hl.width; ++x) {
            hl_energy += std::fabs(hl.data[y * hl.stride + x]);
            lh_energy += std::fabs(lh.data[y * lh.stride + x]);
        }

    assert(lh_energy > hl_energy);

    printf("[PASS] Horizontal edge → LH energy\n");
}

void test_both_edge() {
    const uint32_t W = 32, H = 32;
    std::vector<float> img(W * H, 0.0f);

    for (uint32_t y = H / 2; y < H; ++y)
        for (uint32_t x = 0; x < W; ++x)
            img[y * W + x] = 100.0f;

    dwt2_haar(img.data(), W, H);

    auto hl = HL2(img.data(), W, H);
    auto lh = LH2(img.data(), W, H);

    float hl_energy = 0.0f;
    float lh_energy = 0.0f;

    for (uint32_t y = 0; y < hl.height; ++y)
        for (uint32_t x = 0; x < hl.width; ++x) {
            hl_energy += std::fabs(hl.data[y * hl.stride + x]);
            lh_energy += std::fabs(lh.data[y * lh.stride + x]);
        }

    assert(nearly_equal(lh_energy, hl_energy));

    printf("[PASS] Horizontal edge  ~ LH energy\n");
}

// ----------------------------
// Main
// ----------------------------
int main() {
    test_subband_dimensions();
    test_constant_image_subbands();
    test_both_edge();

    printf("All subband tests passed.\n");
    return 0;
}
