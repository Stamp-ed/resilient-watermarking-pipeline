#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include "wm/transform/dwt.h"
#include "wm/transform/subband.h"
#include "wm/transform/dct_subband.h"

using namespace wm;

static bool nearly_equal(float a, float b, float eps = 1e-3f) {
    return std::fabs(a - b) < eps;
}

void test_dwt_dct_roundtrip() {
    const uint32_t W = 64, H = 64;
    std::vector<float> img(W * H);

    for (uint32_t i = 0; i < W * H; ++i)
        img[i] = static_cast<float>((i * 7) % 31);

    std::vector<float> original = img;

    // Forward DWT
    dwt2_haar(img.data(), W, H);

    // Apply DCT round-trip only on HL₂ and LH₂
    dct_roundtrip_subband(HL2(img.data(), W, H));
    dct_roundtrip_subband(LH2(img.data(), W, H));

    // Inverse DWT
    idwt2_haar(img.data(), W, H);

    for (uint32_t i = 0; i < W * H; ++i)
        assert(nearly_equal(img[i], original[i]));

    printf("[PASS] DWT + DCT subband round-trip\n");
}

int main() {
    test_dwt_dct_roundtrip();
    printf("All DWT+DCT pipeline tests passed.\n");
    return 0;
}
