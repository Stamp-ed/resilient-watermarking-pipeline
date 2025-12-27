#include <cstdio>
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstring>

#include "wm/api.h"

// Helper: RGB to luminance
std::vector<float> rgb_to_luminance(
    const uint8_t* rgb_data,
    uint32_t width,
    uint32_t height
) {
    std::vector<float> luminance(width * height);
    for (uint32_t i = 0; i < width * height; ++i) {
        uint32_t idx = i * 3;
        float r = rgb_data[idx + 0];
        float g = rgb_data[idx + 1];
        float b = rgb_data[idx + 2];
        luminance[i] = 0.299f * r + 0.587f * g + 0.114f * b;
    }
    return luminance;
}

// Helper: Luminance back to RGB (grayscale)
std::vector<uint8_t> luminance_to_rgb(const std::vector<float>& luminance, uint32_t width, uint32_t height) {
    std::vector<uint8_t> rgb(width * height * 3);
    for (uint32_t i = 0; i < width * height; ++i) {
        float lum = std::max(0.0f, std::min(255.0f, luminance[i]));
        uint8_t val = (uint8_t)lum;
        rgb[i * 3 + 0] = val;  // R
        rgb[i * 3 + 1] = val;  // G
        rgb[i * 3 + 2] = val;  // B
    }
    return rgb;
}

// Helper: Pad image
std::vector<float> pad_image(const std::vector<float>& img, uint32_t width, uint32_t height,
                             uint32_t& new_width, uint32_t& new_height) {
    new_width = ((width + 31) / 32) * 32;
    new_height = ((height + 31) / 32) * 32;
    
    if (new_width == width && new_height == height) {
        return img;
    }
    
    std::vector<float> padded(new_width * new_height, 0.0f);
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            padded[y * new_width + x] = img[y * width + x];
        }
    }
    return padded;
}

// Load 24-bit BMP
bool load_bmp(const char* filename, std::vector<uint8_t>& rgb_data, 
              uint32_t& width, uint32_t& height) {
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    
    uint8_t header[54];
    if (fread(header, 1, 54, f) != 54) {
        fclose(f);
        return false;
    }
    
    if (header[0] != 'B' || header[1] != 'M') {
        fclose(f);
        return false;
    }
    
    width = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
    height = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
    
    uint16_t bpp = header[28] | (header[29] << 8);
    if (bpp != 24) {
        fclose(f);
        return false;
    }
    
    uint32_t row_size = ((width * 3 + 3) / 4) * 4;
    uint32_t data_size = row_size * height;
    uint32_t data_offset = header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);
    
    fseek(f, data_offset, SEEK_SET);
    std::vector<uint8_t> raw_data(data_size);
    if (fread(raw_data.data(), 1, data_size, f) != data_size) {
        fclose(f);
        return false;
    }
    
    fclose(f);
    
    rgb_data.resize(width * height * 3);
    for (uint32_t y = 0; y < height; ++y) {
        uint32_t src_row = (height - 1 - y) * row_size;
        uint32_t dst_row = y * width * 3;
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t src_idx = src_row + x * 3;
            uint32_t dst_idx = dst_row + x * 3;
            rgb_data[dst_idx + 0] = raw_data[src_idx + 2];
            rgb_data[dst_idx + 1] = raw_data[src_idx + 1];
            rgb_data[dst_idx + 2] = raw_data[src_idx + 0];
        }
    }
    return true;
}

// Save 24-bit BMP (grayscale)
bool save_bmp(const char* filename, const std::vector<uint8_t>& rgb_data, 
              uint32_t width, uint32_t height) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    
    uint32_t row_size = ((width * 3 + 3) / 4) * 4;
    uint32_t data_size = row_size * height;
    uint32_t file_size = 54 + data_size;
    
    // BMP header
    uint8_t header[54] = {0};
    header[0] = 'B';
    header[1] = 'M';
    header[2] = file_size & 0xFF;
    header[3] = (file_size >> 8) & 0xFF;
    header[4] = (file_size >> 16) & 0xFF;
    header[5] = (file_size >> 24) & 0xFF;
    header[10] = 54;  // Data offset
    header[14] = 40;  // Header size
    header[18] = width & 0xFF;
    header[19] = (width >> 8) & 0xFF;
    header[20] = (width >> 16) & 0xFF;
    header[21] = (width >> 24) & 0xFF;
    header[22] = height & 0xFF;
    header[23] = (height >> 8) & 0xFF;
    header[24] = (height >> 16) & 0xFF;
    header[25] = (height >> 24) & 0xFF;
    header[26] = 1;   // Planes
    header[28] = 24;  // Bits per pixel
    
    fwrite(header, 1, 54, f);
    
    // Pixel data (BMP is bottom-up, BGR)
    std::vector<uint8_t> row(row_size, 0);
    for (int y = height - 1; y >= 0; --y) {
        uint32_t src_row = y * width * 3;
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t src_idx = src_row + x * 3;
            uint32_t dst_idx = x * 3;
            row[dst_idx + 0] = rgb_data[src_idx + 2];  // B
            row[dst_idx + 1] = rgb_data[src_idx + 1];  // G
            row[dst_idx + 2] = rgb_data[src_idx + 0];  // R
        }
        fwrite(row.data(), 1, row_size, f);
    }
    
    fclose(f);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <input.bmp> <output.bmp> [key] [alpha]\n", argv[0]);
        printf("\n");
        printf("This program:\n");
        printf("  1. Loads an image\n");
        printf("  2. Embeds a watermark\n");
        printf("  3. Saves the watermarked image\n");
        printf("\n");
        printf("Note: Output will be grayscale (luminance channel only)\n");
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = argv[2];
    uint64_t key = (argc >= 4) ? strtoull(argv[3], nullptr, 0) : 0xABCDEF1234567890ULL;
    float alpha = (argc >= 5) ? (float)atof(argv[4]) : 9.0f;
    
    printf("=== Save Watermarked Image ===\n\n");
    
    // Load image
    printf("Loading: %s\n", input_file);
    std::vector<uint8_t> rgb_data;
    uint32_t orig_width, orig_height;
    
    if (!load_bmp(input_file, rgb_data, orig_width, orig_height)) {
        printf("Error: Failed to load image\n");
        return 1;
    }
    
    printf("  Size: %dx%d\n", orig_width, orig_height);
    
    // Convert to luminance
    printf("Converting to luminance...\n");
    std::vector<float> original_luminance = rgb_to_luminance(rgb_data.data(), orig_width, orig_height);
    
    // Pad
    uint32_t width, height;
    std::vector<float> padded_luminance = pad_image(original_luminance, orig_width, orig_height, width, height);
    
    if (width != orig_width || height != orig_height) {
        printf("  Padded to: %dx%d\n", width, height);
    }
    
    // Calculate payload
    uint32_t max_payload = (width / 32) * (height / 32) * 2 / 8;
    uint32_t PAYLOAD_LEN = (max_payload > 64) ? 64 : max_payload;
    
    printf("  Payload: %d bits\n", PAYLOAD_LEN);
    
    // Create payload
    std::vector<int8_t> payload_bits(PAYLOAD_LEN);
    for (uint32_t i = 0; i < PAYLOAD_LEN; ++i) {
        payload_bits[i] = (i & 1) ? +1 : -1;
    }
    
    WM_Payload payload;
    payload.bits = payload_bits.data();
    payload.length = PAYLOAD_LEN;
    
    // Embed watermark
    printf("\nEmbedding watermark...\n");
    printf("  Key: 0x%llX\n", key);
    printf("  Alpha: %.2f\n", alpha);
    
    std::vector<float> watermarked_luminance = padded_luminance;
    
    WM_Image img;
    img.width = width;
    img.height = height;
    img.y = watermarked_luminance.data();
    
    WM_Status status = wm_embed(&img, &payload, key, alpha);
    
    if (status != WM_OK) {
        printf("Error: Embedding failed (status %d)\n", status);
        return 1;
    }
    
    // Calculate statistics
    float max_diff = 0.0f;
    double sum_diff_sq = 0.0;
    for (uint32_t i = 0; i < width * height; ++i) {
        float diff = watermarked_luminance[i] - padded_luminance[i];
        float abs_diff = std::fabs(diff);
        if (abs_diff > max_diff) max_diff = abs_diff;
        sum_diff_sq += diff * diff;
    }
    double mse = sum_diff_sq / (width * height);
    double psnr = (mse == 0.0) ? 100.0 : 10.0 * std::log10((255.0 * 255.0) / mse);
    
    printf("  Max pixel change: %.4f\n", max_diff);
    printf("  PSNR: %.2f dB\n", psnr);
    printf("  ✓ Watermark embedded!\n");
    
    // Convert back to RGB and crop to original size
    printf("\nSaving watermarked image...\n");
    std::vector<float> cropped_watermarked(orig_width * orig_height);
    for (uint32_t y = 0; y < orig_height; ++y) {
        for (uint32_t x = 0; x < orig_width; ++x) {
            cropped_watermarked[y * orig_width + x] = watermarked_luminance[y * width + x];
        }
    }
    
    std::vector<uint8_t> output_rgb = luminance_to_rgb(cropped_watermarked, orig_width, orig_height);
    
    if (!save_bmp(output_file, output_rgb, orig_width, orig_height)) {
        printf("Error: Failed to save image\n");
        return 1;
    }
    
    printf("  Saved to: %s\n", output_file);
    
    // Verify extraction
    printf("\nVerifying watermark...\n");
    img.y = watermarked_luminance.data();
    
    std::vector<int8_t> extracted_bits(PAYLOAD_LEN);
    std::vector<float> confidence(PAYLOAD_LEN);
    
    WM_ExtractResult result;
    result.bits = extracted_bits.data();
    result.confidence = confidence.data();
    result.length = PAYLOAD_LEN;
    
    status = wm_extract(&img, key, &result);
    
    if (status == WM_OK) {
        int errors = 0;
        for (uint32_t i = 0; i < PAYLOAD_LEN; ++i) {
            if (payload_bits[i] != extracted_bits[i]) errors++;
        }
        
        printf("  Verdict: ");
        switch (result.verdict) {
            case WM_VERDICT_VERIFIED: printf("VERIFIED\n"); break;
            case WM_VERDICT_TAMPERED: printf("TAMPERED\n"); break;
            case WM_VERDICT_UNVERIFIABLE: printf("UNVERIFIABLE\n"); break;
        }
        printf("  Mean confidence: %.4f\n", result.mean_confidence);
        printf("  Bit errors: %d/%d (%.2f%%)\n", errors, PAYLOAD_LEN, 100.0f * errors / PAYLOAD_LEN);
        
        if (errors == 0) {
            printf("\n✓✓✓ SUCCESS: Watermark verified in saved image! ✓✓✓\n");
        }
    }
    
    printf("\n=== Summary ===\n");
    printf("Original:  %s\n", input_file);
    printf("Watermarked: %s\n", output_file);
    printf("\nTo verify, compare the images or extract watermark from the saved file.\n");
    
    return 0;
}

