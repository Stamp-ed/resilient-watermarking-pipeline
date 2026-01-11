// include/wm/v2/constants.h
#pragma once

namespace wm::v2 {

constexpr int PATCH_SIZE = 48;
constexpr int DWT_LEVELS = 3;
constexpr int DCT_SIZE  = 8;

constexpr float QIM_BASE_STEP = 1.0f;

// Subbands used
enum class Band {
    HL2,
    LH2,
    HL1,
    LH1
};

}
