#include <cstdio>
#include <vector>
#include <cstdint>
#include <string>
#include <algorithm>
#include <cstring>

#include "wm/api.h"

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

// ==================================================
// CONFIG (LOCKED PAYLOAD SIZE)
// ==================================================
constexpr uint32_t PAYLOAD_CHARS = 16;
constexpr uint32_t PAYLOAD_BITS = PAYLOAD_CHARS * 8;

// --------------------------------------------------
// BMP loader (24-bit)
// --------------------------------------------------
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

// --------------------------------------------------
// RGB → Y
// --------------------------------------------------
struct YCbCr
{
    std::vector<float> Y;
    std::vector<float> Cb;
    std::vector<float> Cr;
};

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

// --------------------------------------------------
// Padding helper
// --------------------------------------------------
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

// --------------------------------------------------
// Fixed bits → text
// --------------------------------------------------
std::string fixed_bits_to_text(const std::vector<int8_t> &bits)
{
    std::string out;
    out.reserve(PAYLOAD_CHARS);

    for (uint32_t i = 0; i < PAYLOAD_CHARS; ++i)
    {
        uint8_t ch = 0;
        for (int b = 0; b < 8; ++b)
        {
            if (bits[i * 8 + b] > 0)
                ch |= (1 << (7 - b));
        }
        out.push_back(ch ? (char)ch : '_');
    }
    return out;
}

// --------------------------------------------------
// MAIN
// --------------------------------------------------
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Usage: %s image.bmp key\n", argv[0]);
        printf("Fixed payload size: %u characters\n", PAYLOAD_CHARS);
        return 1;
    }

    const char *in = argv[1];
    uint64_t key = strtoull(argv[2], nullptr, 0);

    // Load image
    std::vector<uint8_t> rgb;
    uint32_t w, h;
    if (!load_bmp(in, rgb, w, h))
    {
        printf("Failed to load BMP\n");
        return 1;
    }

    // RGB → Y
    YCbCr ycbcr = rgb_to_ycbcr(rgb);
    std::vector<float> &Y = ycbcr.Y;

    // Pad
    uint32_t pw, ph;
    std::vector<float> padded_Y = pad_Y(Y, w, h, pw, ph);

    // Capacity check
    uint32_t capacity_bits = 2 * (pw / 32) * (ph / 32);
    if (capacity_bits < PAYLOAD_BITS)
    {
        printf("Image too small for fixed payload (%u bits)\n", PAYLOAD_BITS);
        return 1;
    }

    // Extract
    std::vector<int8_t> bits(PAYLOAD_BITS);
    std::vector<float> conf(PAYLOAD_BITS);

    WM_Image img{pw, ph, padded_Y.data()};
    WM_ExtractResult res{
        bits.data(),
        conf.data(),
        PAYLOAD_BITS};
    printf("Capacity bits: %u\n", capacity_bits);
    printf("Payload bits:  %u\n", PAYLOAD_BITS);
    printf("Blocks/bit:    %.2f\n",
           (double)capacity_bits / PAYLOAD_BITS);
    
    if (wm_extract(&img, key, &res) != WM_OK ||
        res.mean_confidence < 0.7f)
    {
        printf("Failed to reliably extract watermark\n");
        printf("Mean confidence: %.3f\n", res.mean_confidence);
        return 1;
    }

    std::string text = fixed_bits_to_text(bits);
    printf("Extracted text (%u chars): \"%s\"\n",
           PAYLOAD_CHARS, text.c_str());
    printf("Mean confidence: %.3f\n", res.mean_confidence);

    return 0;
}
