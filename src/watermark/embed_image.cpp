#include "wm/watermark/embed_image.h"

#include "wm/transform/dwt.h"
#include "wm/transform/subband.h"
#include "wm/watermark/block_permutation.h"
#include "wm/watermark/embed_block.h"

#include <vector>

namespace wm {

bool embed_image(
    Image& img,
    const int8_t* payload_bits,
    uint32_t payload_len,
    uint64_t key,
    float alpha
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
    // Embed payload
    // -------------------------
    for (uint32_t bit = 0; bit < payload_len; ++bit) {
        for (uint32_t k = 0; k < blocks_per_bit; ++k) {
            uint32_t p = perm[bit * blocks_per_bit + k];

            bool is_lh = (p >= blocks_per_band);
            uint32_t local = is_lh ? (p - blocks_per_band) : p;

            uint32_t by = local / blocks_x;
            uint32_t bx = local % blocks_x;

            SubbandView& band = is_lh ? lh : hl;

            float* block_ptr =
                band.data +
                by * 8 * band.stride +
                bx * 8;

            embed_bit_block(
                block_ptr,
                band.stride,
                payload_bits[bit],
                key,
                bit,
                p,
                alpha
            );
        }
    }

    // -------------------------
    // Inverse DWT
    // -------------------------
    idwt2_haar(img.Y, W, H);

    return true;
}

}
