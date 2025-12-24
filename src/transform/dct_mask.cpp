#include "wm/transform/dct_mask.h"

namespace wm {

const DCTIndex DCT_MID_FREQ_MASK[DCT_MASK_SIZE] = {
    {1, 2},
    {2, 1},
    {2, 2},
    {1, 3},
    {3, 1},
    {2, 3},
    {3, 2}
};

} // namespace wm
