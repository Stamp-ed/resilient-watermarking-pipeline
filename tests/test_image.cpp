#include <cassert>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "wm/image.h"
#include "wm/types.h"
#include "wm/errors.h"

using namespace wm;

// ----------------------------
// Helper to build test image
// ----------------------------
WM_ImageBuffer make_test_image(uint32_t w, uint32_t h) {
    WM_ImageBuffer img;
    img.width = w;
    img.height = h;
    img.channels = 3;
    img.data = new uint8_t[w * h * 3];
    return img;
}

void free_test_image(WM_ImageBuffer& img) {
    delete[] img.data;
    img.data = nullptr;
}

// ----------------------------
// Test 1: Validation
// ----------------------------
void test_validate_image() {
    WM_ImageBuffer img{};
    assert(validate_image(nullptr) == WM_ERR_INVALID_IMAGE);

    img.width = 0;
    img.height = 0;
    img.channels = 3;
    img.data = nullptr;
    assert(validate_image(&img) == WM_ERR_INVALID_IMAGE);

    img.width = 2;
    img.height = 2;
    img.channels = 4;
    img.data = new uint8_t[2 * 2 * 4];
    assert(validate_image(&img) == WM_ERR_UNSUPPORTED_FORMAT);
    delete[] img.data;

    img = make_test_image(2, 2);
    assert(validate_image(&img) == WM_OK);
    free_test_image(img);

    printf("[PASS] test_validate_image\n");
}

// ----------------------------
// Test 2: Luminance correctness
// ----------------------------
void test_luminance_conversion() {
    WM_ImageBuffer img = make_test_image(1, 1);

    // Pure red pixel
    img.data[0] = 255; // R
    img.data[1] = 0;   // G
    img.data[2] = 0;   // B

    Image Y = to_luminance(&img);

    // Expected ≈ 0.299 * 255
    float expected = 0.299f * 255.0f;
    float diff = std::abs(Y.Y[0] - expected);

    assert(diff < 1e-3f);

    free_image(Y);
    free_test_image(img);

    printf("[PASS] test_luminance_conversion\n");
}

// ----------------------------
// Test 3: Memory safety (smoke)
// ----------------------------
void test_memory_safety() {
    WM_ImageBuffer img = make_test_image(32, 32);

    std::memset(img.data, 128, 32 * 32 * 3);

    Image Y = to_luminance(&img);
    assert(Y.Y != nullptr);

    free_image(Y);
    free_test_image(img);

    printf("[PASS] test_memory_safety\n");
}

// ----------------------------
// Main
// ----------------------------
int main() {
    test_validate_image();
    test_luminance_conversion();
    test_memory_safety();

    printf("All image tests passed.\n");
    return 0;
}
