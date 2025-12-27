#include <cstdio>
#include <vector>
#include <cstdint>
#include <cmath>
#include <algorithm>

// Simple difference visualization tool
// This helps visualize where the watermark was embedded

bool load_bmp(const char* filename, std::vector<uint8_t>& rgb_data, 
              uint32_t& width, uint32_t& height) {
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    
    uint8_t header[54];
    if (fread(header, 1, 54, f) != 54) { fclose(f); return false; }
    if (header[0] != 'B' || header[1] != 'M') { fclose(f); return false; }
    
    width = header[18] | (header[19] << 8) | (header[20] << 16) | (header[21] << 24);
    height = header[22] | (header[23] << 8) | (header[24] << 16) | (header[25] << 24);
    
    uint16_t bpp = header[28] | (header[29] << 8);
    if (bpp != 24) { fclose(f); return false; }
    
    uint32_t row_size = ((width * 3 + 3) / 4) * 4;
    uint32_t data_size = row_size * height;
    uint32_t data_offset = header[10] | (header[11] << 8) | (header[12] << 16) | (header[13] << 24);
    
    fseek(f, data_offset, SEEK_SET);
    std::vector<uint8_t> raw_data(data_size);
    if (fread(raw_data.data(), 1, data_size, f) != data_size) { fclose(f); return false; }
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

bool save_bmp(const char* filename, const std::vector<uint8_t>& rgb_data, 
              uint32_t width, uint32_t height) {
    FILE* f = fopen(filename, "wb");
    if (!f) return false;
    
    uint32_t row_size = ((width * 3 + 3) / 4) * 4;
    uint32_t data_size = row_size * height;
    uint32_t file_size = 54 + data_size;
    
    uint8_t header[54] = {0};
    header[0] = 'B'; header[1] = 'M';
    header[2] = file_size & 0xFF;
    header[3] = (file_size >> 8) & 0xFF;
    header[4] = (file_size >> 16) & 0xFF;
    header[5] = (file_size >> 24) & 0xFF;
    header[10] = 54;
    header[14] = 40;
    header[18] = width & 0xFF;
    header[19] = (width >> 8) & 0xFF;
    header[20] = (width >> 16) & 0xFF;
    header[21] = (width >> 24) & 0xFF;
    header[22] = height & 0xFF;
    header[23] = (height >> 8) & 0xFF;
    header[24] = (height >> 16) & 0xFF;
    header[25] = (height >> 24) & 0xFF;
    header[26] = 1;
    header[28] = 24;
    
    fwrite(header, 1, 54, f);
    
    std::vector<uint8_t> row(row_size, 0);
    for (int y = height - 1; y >= 0; --y) {
        uint32_t src_row = y * width * 3;
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t src_idx = src_row + x * 3;
            uint32_t dst_idx = x * 3;
            row[dst_idx + 0] = rgb_data[src_idx + 2];
            row[dst_idx + 1] = rgb_data[src_idx + 1];
            row[dst_idx + 2] = rgb_data[src_idx + 0];
        }
        fwrite(row.data(), 1, row_size, f);
    }
    
    fclose(f);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("Usage: %s <original.bmp> <watermarked.bmp> <difference.bmp> [scale]\n", argv[0]);
        printf("\n");
        printf("Creates a difference image showing where changes occurred.\n");
        printf("Scale factor (default 10.0) - higher = more visible differences\n");
        return 1;
    }
    
    float scale = (argc >= 5) ? (float)atof(argv[4]) : 10.0f;
    
    printf("=== Create Difference Image ===\n\n");
    
    // Load both images
    std::vector<uint8_t> orig_rgb, wm_rgb;
    uint32_t orig_w, orig_h, wm_w, wm_h;
    
    printf("Loading original: %s\n", argv[1]);
    if (!load_bmp(argv[1], orig_rgb, orig_w, orig_h)) {
        printf("Error: Failed to load original image\n");
        return 1;
    }
    
    printf("Loading watermarked: %s\n", argv[2]);
    if (!load_bmp(argv[2], wm_rgb, wm_w, wm_h)) {
        printf("Error: Failed to load watermarked image\n");
        return 1;
    }
    
    if (orig_w != wm_w || orig_h != wm_h) {
        printf("Error: Images must be the same size\n");
        return 1;
    }
    
    printf("  Size: %dx%d\n", orig_w, orig_h);
    
    // Create difference image
    printf("Calculating differences (scale: %.1fx)...\n", scale);
    
    std::vector<uint8_t> diff_rgb(orig_w * orig_h * 3);
    float max_diff = 0.0f;
    
    for (uint32_t i = 0; i < orig_w * orig_h; ++i) {
        // Convert to grayscale (use red channel as luminance approximation)
        float orig_val = orig_rgb[i * 3];
        float wm_val = wm_rgb[i * 3];
        
        float diff = (wm_val - orig_val) * scale;
        float abs_diff = std::fabs(diff);
        if (abs_diff > max_diff) max_diff = abs_diff;
        
        // Map difference to 0-255 range (centered at 128 for no change)
        int diff_val = (int)(128.0f + diff);
        diff_val = std::max(0, std::min(255, diff_val));
        
        diff_rgb[i * 3 + 0] = diff_val;
        diff_rgb[i * 3 + 1] = diff_val;
        diff_rgb[i * 3 + 2] = diff_val;
    }
    
    printf("  Max scaled difference: %.2f\n", max_diff);
    
    // Save difference image
    printf("Saving difference image: %s\n", argv[3]);
    if (!save_bmp(argv[3], diff_rgb, orig_w, orig_h)) {
        printf("Error: Failed to save difference image\n");
        return 1;
    }
    
    printf("\n✓ Difference image created!\n");
    printf("\nInterpretation:\n");
    printf("  - Gray (128) = No change\n");
    printf("  - Brighter = Increased (watermark added)\n");
    printf("  - Darker = Decreased (watermark subtracted)\n");
    printf("  - Scale factor %.1fx makes small changes visible\n", scale);
    
    return 0;
}

