#pragma once
#include <cstdint>

namespace wm {

// In-place 2-level Haar DWT
void dwt2_haar(float* data, uint32_t width, uint32_t height);

// In-place inverse 2-level Haar DWT
void idwt2_haar(float* data, uint32_t width, uint32_t height);

} // namespace wm
