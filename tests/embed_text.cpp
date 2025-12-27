#include <cstdio>
#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>

#include "wm/api.h"
struct YCbCr
{
    std::vector<float> Y;
    std::vector<float> Cb;
    std::vector<float> Cr;
};

// =======================
// BMP (24-bit only)
// =======================

#pragma pack(push, 1)
struct BMPHeader
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BMPInfoHeader
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

bool load_bmp(const char *path, std::vector<uint8_t> &rgb,
              uint32_t &w, uint32_t &h)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return false;

    BMPHeader hdr;
    BMPInfoHeader info;
    fread(&hdr, sizeof(hdr), 1, f);
    fread(&info, sizeof(info), 1, f);

    if (hdr.bfType != 0x4D42 || info.biBitCount != 24)
    {
        fclose(f);
        return false;
    }

    w = info.biWidth;
    h = std::abs(info.biHeight);
    rgb.resize(w * h * 3);

    fseek(f, hdr.bfOffBits, SEEK_SET);

    uint32_t row_padded = (w * 3 + 3) & ~3;
    std::vector<uint8_t> row(row_padded);

    for (uint32_t y = 0; y < h; ++y)
    {
        fread(row.data(), 1, row_padded, f);
        memcpy(&rgb[(h - 1 - y) * w * 3], row.data(), w * 3);
    }

    fclose(f);
    return true;
}

bool save_bmp(const char *path, const std::vector<uint8_t> &rgb,
              uint32_t w, uint32_t h)
{
    FILE *f = fopen(path, "wb");
    if (!f)
        return false;

    uint32_t row_padded = (w * 3 + 3) & ~3;
    uint32_t img_size = row_padded * h;

    BMPHeader hdr{0x4D42, 54 + img_size, 0, 0, 54};
    BMPInfoHeader info{};
    info.biSize = 40;
    info.biWidth = w;
    info.biHeight = h;
    info.biPlanes = 1;
    info.biBitCount = 24;
    info.biSizeImage = img_size;

    fwrite(&hdr, sizeof(hdr), 1, f);
    fwrite(&info, sizeof(info), 1, f);

    std::vector<uint8_t> row(row_padded, 0);
    for (uint32_t y = 0; y < h; ++y)
    {
        memcpy(row.data(), &rgb[(h - 1 - y) * w * 3], w * 3);
        fwrite(row.data(), 1, row_padded, f);
    }

    fclose(f);
    return true;
}

// =======================
// Color conversion
// =======================

YCbCr rgb_to_ycbcr(const std::vector<uint8_t> &rgb)
{
    size_t n = rgb.size() / 3;

    YCbCr out;
    out.Y.resize(n);
    out.Cb.resize(n);
    out.Cr.resize(n);

    for (size_t i = 0; i < n; ++i)
    {
        float R = rgb[3 * i + 0];
        float G = rgb[3 * i + 1];
        float B = rgb[3 * i + 2];

        out.Y[i] = 0.299f * R + 0.587f * G + 0.114f * B;
        out.Cb[i] = -0.168736f * R - 0.331264f * G + 0.5f * B + 128.0f;
        out.Cr[i] = 0.5f * R - 0.418688f * G - 0.081312f * B + 128.0f;
    }

    return out;
}
void ycbcr_to_rgb(std::vector<uint8_t> &rgb, const YCbCr &img)
{
    auto clamp = [](float v)
    {
        return static_cast<uint8_t>(
            std::max(0.f, std::min(255.f, v)));
    };

    size_t n = img.Y.size();

    for (size_t i = 0; i < n; ++i)
    {
        float Y = img.Y[i];
        float Cb = img.Cb[i] - 128.0f;
        float Cr = img.Cr[i] - 128.0f;

        float R = Y + 1.402f * Cr;
        float G = Y - 0.344136f * Cb - 0.714136f * Cr;
        float B = Y + 1.772f * Cb;

        rgb[3 * i + 0] = clamp(R);
        rgb[3 * i + 1] = clamp(G);
        rgb[3 * i + 2] = clamp(B);
    }
}

// =======================
// CONFIG (LOCKED PROTOCOL)
// =======================

constexpr uint32_t PAYLOAD_CHARS = 16;
constexpr uint32_t PAYLOAD_BITS = PAYLOAD_CHARS * 8;

// =======================
// Text → fixed bits
// =======================

std::vector<int8_t> text_to_fixed_bits(const std::string &text)
{
    if (text.size() > PAYLOAD_CHARS)
    {
        printf(
            "ERROR: Text too long. Max allowed = %u characters\n",
            PAYLOAD_CHARS);
        exit(1);
    }

    std::vector<int8_t> bits;
    bits.reserve(PAYLOAD_BITS);

    // Encode characters
    for (size_t i = 0; i < PAYLOAD_CHARS; ++i)
    {
        uint8_t c = (i < text.size()) ? (uint8_t)text[i] : 0;

        for (int b = 7; b >= 0; --b)
            bits.push_back((c >> b) & 1 ? +1 : -1);
    }

    return bits;
}
std::vector<float> pad_Y(
    const std::vector<float> &Y,
    uint32_t w,
    uint32_t h,
    uint32_t &pw,
    uint32_t &ph)
{
    pw = ((w + 31) / 32) * 32;
    ph = ((h + 31) / 32) * 32;

    if (pw == w && ph == h)
        return Y;

    std::vector<float> out(pw * ph, 0.0f);
    for (uint32_t y = 0; y < h; ++y)
        memcpy(&out[y * pw], &Y[y * w], w * sizeof(float));

    return out;
}

// =======================
// Main
// =======================

int main(int argc, char **argv)
{
    if (argc < 6)
    {
        printf("Usage: %s in.bmp out.bmp \"text\"(Size at max 16 char) key alpha\n", argv[0]);
        return 1;
    }

    const char *in = argv[1];
    const char *out = argv[2];
    std::string text = argv[3];
    uint64_t key = strtoull(argv[4], nullptr, 0);
    float alpha = atof(argv[5]);

    std::vector<uint8_t> rgb;
    uint32_t w, h;
    if (!load_bmp(in, rgb, w, h))
    {
        printf("Failed to load BMP\n");
        return 1;
    }
    auto bits = text_to_fixed_bits(text);
    WM_Payload payload{bits.data(), (uint32_t)bits.size()};

    YCbCr img = rgb_to_ycbcr(rgb);

    // Pad Y
    uint32_t pw, ph;
    std::vector<float> padded_Y = pad_Y(img.Y, w, h, pw, ph);

    // Embed
    WM_Image wm_img{pw, ph, padded_Y.data()};
    wm_embed(&wm_img, &payload, key, alpha);

    // COPY BACK embedded Y (crop!)
    for (uint32_t y = 0; y < h; ++y)
    {
        memcpy(
            &img.Y[y * w],
            &padded_Y[y * pw],
            w * sizeof(float));
    }

    // Reconstruct RGB from modified Y + original Cb/Cr
    ycbcr_to_rgb(rgb, img);
    save_bmp(out, rgb, w, h);

    printf(
        "Embedded text (fixed %u chars): \"%s\"\n",
        PAYLOAD_CHARS,
        text.c_str());

    return 0;
}
