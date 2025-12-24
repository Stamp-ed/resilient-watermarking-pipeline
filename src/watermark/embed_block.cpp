#include "wm/watermark/embed_block.h"

#include "wm/transform/dct.h"
#include "wm/transform/dct_mask.h"
#include "wm/watermark/pn.h"

namespace wm {

void embed_bit_block(
    float* spatial_block,
    uint32_t stride,
    int8_t bit,
    uint64_t key,
    uint32_t bit_index,
    uint32_t block_index,
    float alpha
) {
    // -----------------------------
    // 1. Load spatial block
    // -----------------------------
    float block[64];
    for (uint32_t y = 0; y < 8; ++y)
        for (uint32_t x = 0; x < 8; ++x)
            block[y * 8 + x] = spatial_block[y * stride + x];

    // -----------------------------
    // 2. Forward DCT
    // -----------------------------
    float coeff[64];
    dct8x8(block, coeff);

    // -----------------------------
    // 3. Spread-spectrum embedding
    // -----------------------------
    for (uint32_t i = 0; i < DCT_MASK_SIZE; ++i) {
        uint8_t u = DCT_MID_FREQ_MASK[i].u;
        uint8_t v = DCT_MID_FREQ_MASK[i].v;

        uint32_t idx = u * 8 + v;

        int8_t pn = pn_chip(
            key,
            bit_index,
            block_index,
            i        // chip index
        );

        coeff[idx] += alpha * float(bit) * float(pn);
    }

    // -----------------------------
    // 4. Inverse DCT
    // -----------------------------
    float recon[64];
    idct8x8(coeff, recon);

    // -----------------------------
    // 5. Store back
    // -----------------------------
    for (uint32_t y = 0; y < 8; ++y)
        for (uint32_t x = 0; x < 8; ++x)
            spatial_block[y * stride + x] = recon[y * 8 + x];
}

} // namespace wm
