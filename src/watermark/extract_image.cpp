#include "wm/watermark/extract_image.h"

#include "wm/transform/dwt.h"
#include "wm/transform/subband.h"
#include "wm/watermark/block_permutation.h"
#include "wm/watermark/extract_block.h"

#include <vector>
#include <cmath>

namespace wm {

bool extract_image(
    Image& img,
    int8_t* bits_out,
    float* confidence_out,
    uint32_t payload_len,
    uint64_t key
) {
    const uint32_t W = img.width;
    const uint32_t H = img.height;

    // -------------------------
    // Validate dimensions
    // -------------------------
    if (W % 32 != 0 || H % 32 != 0)
        return false;

    // -------------------------
    // Forward DWT
    // -------------------------
    dwt2_haar(img.Y, W, H);

    // -------------------------
    // Subbands
    // -------------------------
    SubbandView hl = HL2(img.Y, W, H);
    SubbandView lh = LH2(img.Y, W, H);

    const uint32_t blocks_x = W / 32;
    const uint32_t blocks_y = H / 32;
    const uint32_t blocks_per_band = blocks_x * blocks_y;
    const uint32_t total_blocks = 2 * blocks_per_band;

    if (total_blocks < payload_len)
        return false;

    const uint32_t blocks_per_bit = total_blocks / payload_len;

    // -------------------------
    // Block permutation
    // -------------------------
    std::vector<uint32_t> perm(total_blocks);
    generate_block_permutation(key, perm.data(), total_blocks);

    // -------------------------
    // Extract payload
    // -------------------------
    for (uint32_t bit = 0; bit < payload_len; ++bit) {
        int32_t sum = 0;

        for (uint32_t k = 0; k < blocks_per_bit; ++k) {
            uint32_t p = perm[bit * blocks_per_bit + k];

            bool is_lh = (p >= blocks_per_band);
            uint32_t local = is_lh ? (p - blocks_per_band) : p;

            uint32_t by = local / blocks_x;
            uint32_t bx = local % blocks_x;

            SubbandView& band = is_lh ? lh : hl;

            const float* block_ptr =
                band.data +
                by * 8 * band.stride +
                bx * 8;

            int8_t vote = extract_bit_block(
                block_ptr,
                band.stride,
                key,
                bit,
                p
            );

            sum += vote;
        }

        bits_out[bit] = (sum >= 0) ? +1 : -1;
        confidence_out[bit] =
            std::fabs(static_cast<float>(sum)) /
            static_cast<float>(blocks_per_bit);
    }

    // -------------------------
    // Inverse DWT
    // -------------------------
    idwt2_haar(img.Y, W, H);

    return true;
}

}
