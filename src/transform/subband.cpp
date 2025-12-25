#include "wm/transform/subband.h"
#include <cassert>

namespace wm {

static inline void validate(uint32_t W, uint32_t H) {
    assert(W % 4 == 0 && H % 4 == 0);
}

SubbandView LL2(float* data, uint32_t W, uint32_t H) {
    validate(W, H);
    return { data, W / 4, H / 4, W };
}

SubbandView HL2(float* data, uint32_t W, uint32_t H) {
    validate(W, H);
    return {
        data + (W / 4),
        W / 4,
        H / 4,
        W
    };
}

SubbandView LH2(float* data, uint32_t W, uint32_t H) {
    validate(W, H);
    return {
        data + (H / 4) * W,
        W / 4,
        H / 4,
        W
    };
}

SubbandView HH2(float* data, uint32_t W, uint32_t H) {
    validate(W, H);
    return {
        data + (H / 4) * W + (W / 4),
        W / 4,
        H / 4,
        W
    };
}

} // namespace wm
