    #include <cstdio>
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstring>

// Include the C API header
#include "wm/api.h"

// Helper function to convert RGB bytes to luminance float array
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
        
        // ITU-R BT.601 luminance conversion
        luminance[i] = 0.299f * r + 0.587f * g + 0.114f * b;
    }
    
    return luminance;
}

// Helper to pad image to multiple of 32
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

// Simple BMP loader (24-bit RGB BMP files)
bool load_bmp(const char* filename, std::vector<uint8_t>& rgb_data, 
              uint32_t& width, uint32_t& height) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Error: Cannot open file %s\n", filename);
        return false;
    }
    
    uint8_t header[54];
    if (fread(header, 1, 54, f) != 54) {
        fclose(f);
        return false;
    }
    
    if (header[0] != 'B' || header[1] != 'M') {
        printf("Error: Not a valid BMP file\n");
        fclose(f);
        return false;
    }
    
    width = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
    height = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
    
    uint16_t bpp = header[28] | (header[29] << 8);
    if (bpp != 24) {
        printf("Error: Only 24-bit BMP files supported (this file is %d-bit)\n", bpp);
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

int main(int argc, char* argv[]) {
    printf("=== Real Image Watermarking Test (Improved) ===\n\n");
    
    if (argc < 2) {
        printf("Usage: %s <image_file> [key] [alpha]\n", argv[0]);
        printf("\n");
        printf("Recommended: Use image at least 512x512 pixels for best results\n");
        return 1;
    }
    
    const char* image_file = argv[1];
    uint64_t key = (argc >= 3) ? strtoull(argv[2], nullptr, 0) : 0xABCDEF1234567890ULL;
    float alpha = (argc >= 4) ? (float)atof(argv[3]) : 2.0f;
    
    // ----------------------------
    // Load image
    // ----------------------------
    printf("Loading image: %s\n", image_file);
    std::vector<uint8_t> rgb_data;
    uint32_t orig_width, orig_height;
    
    if (!load_bmp(image_file, rgb_data, orig_width, orig_height)) {
        printf("Failed to load image!\n");
        return 1;
    }
    
    printf("  Original size: %dx%d\n", orig_width, orig_height);
    
    if (orig_width < 256 || orig_height < 256) {
        printf("  WARNING: Image is quite small. Larger images (512x512+) work better.\n");
    }
    
    // ----------------------------
    // Convert to luminance
    // ----------------------------
    printf("Converting to luminance...\n");
    std::vector<float> original_luminance = rgb_to_luminance(rgb_data.data(), orig_width, orig_height);
    
    // ----------------------------
    // Pad to multiple of 32
    // ----------------------------
    uint32_t width, height;
    std::vector<float> padded_luminance = pad_image(original_luminance, orig_width, orig_height, width, height);
    
    if (width != orig_width || height != orig_height) {
        printf("  Padded to: %dx%d (required for watermarking)\n", width, height);
    }
    
    // ----------------------------
    // Prepare watermark payload
    // ----------------------------
    uint32_t max_payload = (width / 32) * (height / 32) * 2 / 8;
    uint32_t PAYLOAD_LEN = (max_payload > 64) ? 64 : max_payload;
    
    if (PAYLOAD_LEN < 8) {
        printf("Error: Image too small! Minimum size: 256x256\n");
        return 1;
    }
    
    printf("  Max payload capacity: %d bits\n", max_payload);
    printf("  Using payload: %d bits\n", PAYLOAD_LEN);
    
    std::vector<int8_t> payload_bits(PAYLOAD_LEN);
    for (uint32_t i = 0; i < PAYLOAD_LEN; ++i) {
        payload_bits[i] = (i & 1) ? +1 : -1;
    }
    
    WM_Payload payload;
    payload.bits = payload_bits.data();
    payload.length = PAYLOAD_LEN;
    
    // ----------------------------
    // EMBED watermark (on COPY of image)
    // ----------------------------
    printf("\nEmbedding watermark...\n");
    printf("  Key: 0x%llX\n", key);
    printf("  Alpha: %.2f\n", alpha);
    
    // IMPORTANT: Make a copy for embedding
    std::vector<float> watermarked_luminance = padded_luminance;
    
    WM_Image img_embed;
    img_embed.width = width;
    img_embed.height = height;
    img_embed.y = watermarked_luminance.data();
    
    WM_Status status = wm_embed(&img_embed, &payload, key, alpha);
    
    if (status != WM_OK) {
        printf("ERROR: Embedding failed with status %d\n", status);
        return 1;
    }
    
    // Check if image was actually modified
    float max_diff = 0.0f;
    for (uint32_t i = 0; i < width * height; ++i) {
        float diff = std::fabs(watermarked_luminance[i] - padded_luminance[i]);
        if (diff > max_diff) max_diff = diff;
    }
    printf("  Max pixel change: %.4f\n", max_diff);
    printf("  Embedding successful!\n");
    
    // ----------------------------
    // EXTRACT watermark (from watermarked image)
    // ----------------------------
    printf("\nExtracting watermark from watermarked image...\n");
    
    std::vector<int8_t> extracted_bits(PAYLOAD_LEN);
    std::vector<float> confidence(PAYLOAD_LEN);
    
    WM_ExtractResult result;
    result.bits = extracted_bits.data();
    result.confidence = confidence.data();
    result.length = PAYLOAD_LEN;
    
    WM_Image img_extract;
    img_extract.width = width;
    img_extract.height = height;
    img_extract.y = watermarked_luminance.data();  // Use watermarked image
    
    status = wm_extract(&img_extract, key, &result);
    
    if (status != WM_OK) {
        printf("ERROR: Extraction failed with status %d\n", status);
        return 1;
    }
    
    printf("  Extraction successful!\n");
    
    // ----------------------------
    // Display results
    // ----------------------------
    printf("\n=== Results ===\n");
    printf("Verdict: ");
    switch (result.verdict) {
        case WM_VERDICT_VERIFIED:
            printf("VERIFIED (Watermark detected successfully!)\n");
            break;
        case WM_VERDICT_TAMPERED:
            printf("TAMPERED (Watermark partially destroyed)\n");
            break;
        case WM_VERDICT_UNVERIFIABLE:
            printf("UNVERIFIABLE (Cannot verify watermark)\n");
            break;
        default:
            printf("UNKNOWN (%d)\n", result.verdict);
    }
    
    printf("Mean confidence: %.4f\n", result.mean_confidence);
    printf("Min confidence:  %.4f\n", result.min_confidence);
    
    // Check bit accuracy
    int errors = 0;
    printf("\nBit accuracy (first 16 bits):\n");
    for (uint32_t i = 0; i < PAYLOAD_LEN && i < 16; ++i) {
        int8_t orig = payload_bits[i];
        int8_t extr = extracted_bits[i];
        bool match = (orig == extr);
        if (!match) errors++;
        
        printf("  Bit %2d: %+3d -> %+3d %s (conf: %.3f)\n",
               i, orig, extr, match ? "OK" : "ERR", confidence[i]);
    }
    if (PAYLOAD_LEN > 16) {
        printf("  ... (%d more bits)\n", PAYLOAD_LEN - 16);
    }
    
    int total_errors = 0;
    for (uint32_t i = 0; i < PAYLOAD_LEN; ++i) {
        if (payload_bits[i] != extracted_bits[i]) total_errors++;
    }
    
    printf("\n=== Summary ===\n");
    printf("Total bits: %d\n", PAYLOAD_LEN);
    printf("Errors: %d\n", total_errors);
    printf("Bit Error Rate: %.2f%%\n", 100.0f * total_errors / PAYLOAD_LEN);
    
    // Interpretation
    printf("\n=== Interpretation ===\n");
    if (result.verdict == WM_VERDICT_VERIFIED && total_errors == 0) {
        printf("PERFECT: Watermarking works perfectly!\n");
        printf("  - All bits extracted correctly\n");
        printf("  - High confidence values\n");
        return 0;
    } else if (result.verdict == WM_VERDICT_VERIFIED && total_errors < PAYLOAD_LEN * 0.1) {
        printf("GOOD: Watermarking works well!\n");
        printf("  - Most bits extracted correctly (%.1f%%)\n", 100.0f * (1.0f - (float)total_errors / PAYLOAD_LEN));
        printf("  - Acceptable for real-world use\n");
        return 0;
    } else if (result.verdict == WM_VERDICT_TAMPERED || result.mean_confidence < 0.6f) {
        printf("ISSUES DETECTED:\n");
        if (orig_width < 512 || orig_height < 512) {
            printf("  - Image may be too small (try 512x512 or larger)\n");
        }
        if (alpha < 1.0f) {
            printf("  - Alpha may be too low (try 2.0 or higher)\n");
        }
        if (result.min_confidence == 0.0f) {
            printf("  - Some bits have zero confidence (image may not have enough detail)\n");
        }
        printf("  - Try with a larger image or different alpha value\n");
        return 1;
    } else {
        printf("UNCLEAR: Results are ambiguous\n");
        return 1;
    }
}

