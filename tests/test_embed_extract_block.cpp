#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

#include "wm/watermark/embed_block.h"
#include "wm/watermark/extract_block.h"

using namespace wm;

static bool nearly_equal(float a, float b, float eps = 1e-3f) {
    return std::fabs(a - b) < eps;
}

// ----------------------------
// Test: single-block embed/extract
// ----------------------------
void test_embed_extract_single_block() {
    constexpr uint32_t STRIDE = 8;
    float block[STRIDE * STRIDE];

    // ----------------------------
    // 1. Initialize smooth block
    // ----------------------------
    for (uint32_t i = 0; i < STRIDE * STRIDE; ++i)
        block[i] = 50.0f;  // flat region

    // Backup original for perceptual check
    float original[64];
    for (int i = 0; i < 64; ++i)
        original[i] = block[i];

    uint64_t key = 0xDEADBEEFCAFEBABEULL;
    uint32_t bit_index = 0;
    uint32_t block_index = 0;
    float alpha = 2.0f;

    // ----------------------------
    // 2. Embed bit +1
    // ----------------------------
    embed_bit_block(
        block,
        STRIDE,
        +1,
        key,
        bit_index,
        block_index,
        alpha
    );

    // ----------------------------
    // 3. Extract bit
    // ----------------------------
    int8_t extracted = extract_bit_block(
        block,
        STRIDE,
        key,
        bit_index,
        block_index
    );

    assert(extracted == +1);

    // ----------------------------
    // 4. Perceptual sanity check
    // ----------------------------
    float max_diff = 0.0f;
    for (int i = 0; i < 64; ++i)
        max_diff = std::max(max_diff,
                            std::fabs(block[i] - original[i]));

    assert(max_diff < 5.0f); // very loose, but should be small

    printf("[PASS] Embed/extract bit +1\n");

    // ----------------------------
    // 5. Noise robustness
    // ----------------------------
    for (int i = 0; i < 64; ++i)
        block[i] += (i % 3 == 0) ? 0.5f : -0.5f;

    extracted = extract_bit_block(
        block,
        STRIDE,
        key,
        bit_index,
        block_index
    );

    assert(extracted == +1);

    printf("[PASS] Noise robustness\n");
}

// ----------------------------
// Test: negative bit
// ----------------------------
void test_embed_extract_negative_bit() {
    constexpr uint32_t STRIDE = 8;
    float block[STRIDE * STRIDE];

    for (uint32_t i = 0; i < STRIDE * STRIDE; ++i)
        block[i] = 50.0f;

    uint64_t key = 0x123456789ABCDEF0ULL;
    uint32_t bit_index = 3;
    uint32_t block_index = 7;
    float alpha = 2.0f;

    embed_bit_block(
        block,
        STRIDE,
        -1,
        key,
        bit_index,
        block_index,
        alpha
    );

    int8_t extracted = extract_bit_block(
        block,
        STRIDE,
        key,
        bit_index,
        block_index
    );

    assert(extracted == -1);

    printf("[PASS] Embed/extract bit -1\n");
}

// ----------------------------
// Main
// ----------------------------
int main() {
    test_embed_extract_single_block();
    test_embed_extract_negative_bit();

    printf("All single-block embed/extract tests passed.\n");
    return 0;
}
