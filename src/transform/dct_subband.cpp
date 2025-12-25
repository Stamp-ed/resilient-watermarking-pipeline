#include "wm/transform/dct_subband.h"
#include "wm/transform/dct.h"
#include "wm/transform/block.h"

namespace wm {

void dct_roundtrip_subband(const SubbandView& band) {
    float block[64];
    float coeff[64];
    float recon[64];

    for_each_block_8x8(band, [&](float* base, uint32_t stride) {
        // Load block
        for (uint32_t y = 0; y < 8; ++y)
            for (uint32_t x = 0; x < 8; ++x)
                block[y * 8 + x] = base[y * stride + x];

        // Forward + inverse DCT
        dct8x8(block, coeff);
        idct8x8(coeff, recon);

        // Store back
        for (uint32_t y = 0; y < 8; ++y)
            for (uint32_t x = 0; x < 8; ++x)
                base[y * stride + x] = recon[y * 8 + x];
    });
}

} // namespace wm
