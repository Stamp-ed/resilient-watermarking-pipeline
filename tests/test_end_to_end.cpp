#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstdint>

#include "wm/image.h"
#include "wm/watermark/embed_image.h"
#include "wm/watermark/extract_image.h"

using namespace wm;

// ----------------------------
// PSNR computation
// ----------------------------
float compute_psnr(const float* orig,
                   const float* wm,
                   uint32_t size)
{
    double mse = 0.0;
    for (uint32_t i = 0; i < size; ++i) {
        double diff = orig[i] - wm[i];
        mse += diff * diff;
    }
    mse /= double(size);

    if (mse == 0.0)
        return 100.0f;

    return float(10.0 * std::log10((255.0 * 255.0) / mse));
}

// ----------------------------
// Main test
// ----------------------------
int main() {
    constexpr uint32_t W = 512;
    constexpr uint32_t H = 512;
    constexpr uint32_t PAYLOAD_LEN = 64;

    // ----------------------------
    // Create synthetic luminance image
    // ----------------------------
    std::vector<float> Y(W * H);

    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            Y[y * W + x] =
                100.0f +
                30.0f * std::sin(0.02f * x) +
                20.0f * std::cos(0.015f * y);
        }
    }

    std::vector<float> original_Y = Y;

    Image img;
    img.width = W;
    img.height = H;
    img.Y = Y.data();

    // ----------------------------
    // Payload
    // ----------------------------
    int8_t payload[PAYLOAD_LEN];
    for (uint32_t i = 0; i < PAYLOAD_LEN; ++i)
        payload[i] = (i & 1) ? +1 : -1;

    uint64_t key = 0xABCDEF1234567890ULL;
    float alpha = 2.0f;

    // ----------------------------
    // Embed
    // ----------------------------
    bool ok = embed_image(
        img,
        payload,
        PAYLOAD_LEN,
        key,
        alpha
    );

    assert(ok);

    // ----------------------------
    // Metrics: PSNR
    // ----------------------------
    float psnr = compute_psnr(
        original_Y.data(),
        img.Y,
        W * H
    );

    printf("PSNR: %.2f dB\n", psnr);
    assert(psnr >= 40.0f);

    // ----------------------------
    // Extract
    // ----------------------------
    int8_t extracted[PAYLOAD_LEN];
    float confidence[PAYLOAD_LEN];

    ok = extract_image(
        img,
        extracted,
        confidence,
        PAYLOAD_LEN,
        key
    );

    assert(ok);

    // ----------------------------
    // BER + confidence
    // ----------------------------
    uint32_t errors = 0;
    float conf_sum = 0.0f;
    float conf_min = 1.0f;

    for (uint32_t i = 0; i < PAYLOAD_LEN; ++i) {
        if (extracted[i] != payload[i])
            errors++;

        conf_sum += confidence[i];
        conf_min = std::min(conf_min, confidence[i]);
    }

    float ber = float(errors) / float(PAYLOAD_LEN);
    float conf_mean = conf_sum / PAYLOAD_LEN;

    printf("BER: %.4f\n", ber);
    printf("Mean confidence: %.3f\n", conf_mean);
    printf("Min confidence:  %.3f\n", conf_min);

    assert(ber == 0.0f);
    assert(conf_min >= 0.6f);

    printf("END-TO-END TEST PASSED\n");
    return 0;
}
