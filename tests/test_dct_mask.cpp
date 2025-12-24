#include <cassert>
#include <cstdio>
#include "wm/transform/dct_mask.h"

using namespace wm;

int main() {
    for (uint32_t i = 0; i < DCT_MASK_SIZE; ++i) {
        assert(DCT_MID_FREQ_MASK[i].u < 8);
        assert(DCT_MID_FREQ_MASK[i].v < 8);
        assert(!(DCT_MID_FREQ_MASK[i].u == 0 &&
                 DCT_MID_FREQ_MASK[i].v == 0));
    }

    printf("[PASS] DCT mid-frequency mask\n");
    return 0;
}
