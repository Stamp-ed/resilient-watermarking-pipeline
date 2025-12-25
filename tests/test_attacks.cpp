#include <vector>
#include <cstdio>
#include <cmath>
#include <cassert>

#include "wm/image.h"
#include "wm/watermark/embed_image.h"
#include "wm/watermark/extract_image.h"


void jpeg_like_attack(float* img, uint32_t W, uint32_t H, float q) {
    // q ∈ [0,1], lower = harsher
    for (uint32_t i = 0; i < W * H; ++i) {
        img[i] = std::round(img[i] * q) / q;
    }
}

void crop_attack(float* img, uint32_t W, uint32_t H, float ratio) {
    uint32_t cx = uint32_t(W * ratio);
    uint32_t cy = uint32_t(H * ratio);

    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            if (x < cx || x >= W - cx ||
                y < cy || y >= H - cy)
                img[y * W + x] = 0.0f;
        }
    }
}

void noise_attack(float* img, uint32_t size, float amp) {
    for (uint32_t i = 0; i < size; ++i)
        img[i] += amp * std::sin(float(i));
}


using namespace wm;

void run_attack(
    const char* name,
    std::vector<float> attacked,
    Image& img,
    const int8_t* payload,
    uint32_t payload_len,
    uint64_t key
) {
    img.Y = attacked.data();

    int8_t extracted[64];
    float confidence[64];

    bool ok = extract_image(
        img,
        extracted,
        confidence,
        payload_len,
        key
    );

    assert(ok);

    uint32_t errors = 0;
    float mean_conf = 0.0f;
    float min_conf = 1.0f;

    for (uint32_t i = 0; i < payload_len; ++i) {
        if (extracted[i] != payload[i])
            errors++;
        mean_conf += confidence[i];
        min_conf = std::min(min_conf, confidence[i]);
    }

    mean_conf /= payload_len;

    printf(
        "%-25s | BER: %.3f | MeanConf: %.2f | MinConf: %.2f\n",
        name,
        float(errors) / payload_len,
        mean_conf,
        min_conf
    );
}

int main() {
    constexpr uint32_t W = 512, H = 512;
    constexpr uint32_t PAYLOAD_LEN = 64;

    std::vector<float> img_buf(W * H);

    for (uint32_t i = 0; i < W * H; ++i)
        img_buf[i] = 100.0f + 20.0f * std::sin(0.01f * i);

    Image img{W, H, img_buf.data()};

    int8_t payload[64];
    for (int i = 0; i < 64; ++i)
        payload[i] = (i & 1) ? +1 : -1;

    uint64_t key = 0xBEEF12345678ULL;
    float alpha = 2.0f;

    embed_image(img, payload, PAYLOAD_LEN, key, alpha);

    std::vector<float> wm = img_buf;

    // ---- Attacks ----
    jpeg_like_attack(wm.data(), W, H, 4.0f);
    run_attack("JPEG Q90", wm, img, payload, PAYLOAD_LEN, key);

    wm = img_buf;
    jpeg_like_attack(wm.data(), W, H, 2.5f);
    run_attack("JPEG Q80", wm, img, payload, PAYLOAD_LEN, key);

    wm = img_buf;
    jpeg_like_attack(wm.data(), W, H, 1.5f);
    run_attack("JPEG Q70", wm, img, payload, PAYLOAD_LEN, key);

    wm = img_buf;
    noise_attack(wm.data(), W * H, 1.0f);
    run_attack("Noise amp=1", wm, img, payload, PAYLOAD_LEN, key);

    wm = img_buf;
    crop_attack(wm.data(), W, H, 0.2f);
    run_attack("Crop 20%", wm, img, payload, PAYLOAD_LEN, key);

    return 0;
}
