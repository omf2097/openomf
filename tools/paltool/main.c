/** @file main.c
 * @brief Palette optimization tool — pack custom colors into portrait slots.
 *
 * Algorithm:
 * 1. Build fixed reference set: base non-slot (192) + ext_common (12) + expanded_common (96) = 300 colors
 * 2. Map each opaque image pixel to nearest reference color. If close enough, it's "matched".
 * 3. Collect unmatched image pixels as "custom" candidates.
 * 4. Collapse custom candidates down to 64 by iteratively merging closest pair (oklab distance).
 * 5. Output: 256-entry palette = 96 expanded_common + 64 custom + 84 BK common + 12 ext_common.
 */

#include "oklab.h"
#include "formats/bk.h"
#include "formats/error.h"
#include "utils/path.h"
#include "video/vga_palette.h"
#include "video/vga_extended_palette.h"
#include <argtable3.h>
#include <math.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SLOT_SIZE 64

// Read PNG into RGBA buffer
static int read_png(const char *path, uint8_t **out_rgba, int *out_w, int *out_h) {
    FILE *fp = fopen(path, "rb");
    if(!fp) { fprintf(stderr, "Cannot open %s\n", path); return -1; }

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);
    if(setjmp(png_jmpbuf(png))) { fclose(fp); return -1; }

    png_init_io(png, fp);
    png_read_info(png, info);
    *out_w = png_get_image_width(png, info);
    *out_h = png_get_image_height(png, info);
    int color_type = png_get_color_type(png, info);
    int bit_depth = png_get_bit_depth(png, info);

    if(bit_depth == 16) png_set_strip_16(png);
    if(color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png);
    if(color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png);
    if(png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
    if(color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png);
    png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);

    png_read_update_info(png, info);
    int rowbytes = png_get_rowbytes(png, info);
    uint8_t *buf = malloc((*out_h) * rowbytes);
    png_bytep *rows = malloc((*out_h) * sizeof(png_bytep));
    for(int y = 0; y < *out_h; y++) rows[y] = buf + y * rowbytes;
    png_read_image(png, rows);
    png_read_end(png, NULL);

    int npixels = (*out_w) * (*out_h);
    *out_rgba = malloc(npixels * 4);
    memcpy(*out_rgba, buf, npixels * 4);

    free(rows);
    free(buf);
    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL);
    return 0;
}

int main(int argc, char *argv[]) {
    struct arg_lit *help = arg_lit0("h", "help", "print this help");
    struct arg_file *input = arg_file1("i", "input", "<png>", "Input PNG image");
    struct arg_file *bkfile = arg_file1("b", "bk", "<bk>", "BK file to load palette from");
    struct arg_file *output = arg_file0("o", "output", "<png>", "Output paletted PNG");
    struct arg_file *imgout = arg_file0("p", "palette", "<bin>", "Output slot palette binary (64 RGB triplets)");
    struct arg_int *slot = arg_int0("s", "slot", "<int>", "Portrait slot index (0-4, default 0)");
    struct arg_dbl *magthresh = arg_dbl0("m", "magenta-thresh", "<int>", "Manhattan distance threshold for magenta key color (default 350)");
    struct arg_lit *verbose = arg_lit0("v", "verbose", "Print detailed progress");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, input, bkfile, output, imgout, slot, magthresh, verbose, end};

    if(arg_nullcheck(argtable) != 0) { printf("Insufficient memory\n"); goto exit; }
    int nerrors = arg_parse(argc, argv, argtable);
    if(help->count > 0) {
        printf("Usage: paltool ");
        arg_print_syntax(stdout, argtable, "\n");
        arg_print_glossary(stdout, argtable, "%-25s %s\n");
        goto exit;
    }
    if(nerrors > 0) {
        arg_print_errors(stdout, end, "paltool");
        printf("Try '--help' for more information.\n");
        goto exit;
    }

    int vflag = verbose->count;
    int mag_thresh = magthresh->count ? (int)magthresh->dval[0] : 350;

    // Read input image
    uint8_t *rgba = NULL;
    int w, h;
    if(read_png(input->filename[0], &rgba, &w, &h) != 0) {
        fprintf(stderr, "Failed to read %s\n", input->filename[0]);
        goto exit;
    }
    int npixels = w * h;
    if(vflag) fprintf(stderr, "Image: %dx%d (%d pixels)\n", w, h, npixels);

    // Load base palette from BK file
    vga_palette vga_pal;
    memset(&vga_pal, 0, sizeof(vga_pal));
    sd_bk_file bk;
    sd_bk_create(&bk);
    path bk_path;
    path_from_c(&bk_path, bkfile->filename[0]);
    if(sd_bk_load(&bk, &bk_path) != SD_SUCCESS) {
        fprintf(stderr, "Failed to load BK file %s\n", bkfile->filename[0]);
        goto exit;
    }
    if(bk.palettes[0]) {
        memcpy(&vga_pal, bk.palettes[0], sizeof(vga_palette));
        if(vflag) fprintf(stderr, "Loaded BK palette (%d palettes in file)\n", bk.palette_count);
    } else {
        fprintf(stderr, "Warning: BK has no palettes\n");
    }
    sd_bk_free(&bk);

    // Fill extended zones with magenta (matches runtime default)
    for(int i = VGA_EXT_COMMON_START; i < VGA_PALETTE_SIZE; i++) {
        vga_pal.colors[i].r = 255;
        vga_pal.colors[i].g = 0;
        vga_pal.colors[i].b = 255;
    }

    // Extended common colors (static, same as vga_state_init)
    static const vga_color ext_common[12] = {
        {40, 28, 16}, {72, 48, 28}, {105, 72, 44}, {140, 97, 60},
        {93, 0, 48}, {157, 24, 85}, {214, 60, 133}, {255, 121, 186},
        {8, 40, 16}, {24, 76, 32}, {48, 113, 56}, {80, 153, 85},
    };
    memcpy(&vga_pal.colors[VGA_EXT_COMMON_START], ext_common, sizeof(ext_common));

    // Expanded common colors (static, same as vga_state_init)
    static const vga_color ext_expanded_common[96] = {
        {0, 93, 0}, {0, 125, 0}, {0, 157, 0}, {0, 190, 0}, {0, 222, 0}, {0, 239, 0}, {0, 206, 0},
        {0, 0, 93}, {0, 0, 125}, {0, 0, 157}, {0, 0, 190}, {0, 0, 222}, {0, 0, 239}, {0, 0, 206},
        {93, 0, 0}, {125, 0, 0}, {157, 0, 0}, {190, 0, 0}, {222, 0, 0}, {239, 0, 0}, {206, 0, 0},
        {75, 0, 93}, {101, 0, 125}, {125, 0, 157}, {147, 0, 190}, {168, 0, 222}, {186, 0, 239}, {157, 0, 206},
        {107, 121, 125}, {129, 145, 149}, {151, 168, 172}, {172, 190, 194}, {192, 214, 218}, {212, 238, 243}, {168, 186, 190},
        {77, 75, 4}, {109, 103, 12}, {141, 129, 24}, {174, 156, 42}, {206, 182, 62}, {239, 208, 87}, {168, 151, 39},
        {16, 105, 125}, {36, 131, 149}, {60, 160, 172}, {89, 186, 194}, {123, 212, 218}, {164, 241, 243}, {83, 182, 190},
        {115, 87, 56}, {143, 107, 70}, {172, 127, 85}, {200, 147, 101}, {220, 172, 125}, {238, 210, 166}, {195, 144, 98},
        {206, 89, 12}, {224, 121, 34}, {242, 153, 56},
        {255, 220, 0}, {255, 235, 0}, {255, 245, 0},
        {255, 250, 32}, {255, 255, 80}, {255, 255, 133},
        {178, 125, 80}, {210, 157, 105}, {235, 190, 140}, {252, 226, 182},
        {255, 153, 202}, {255, 182, 218}, {255, 210, 232}, {255, 234, 246},
        {113, 190, 117}, {153, 222, 153}, {190, 240, 190}, {222, 250, 222},
        {157, 157, 0}, {198, 198, 0}, {238, 238, 0}, {255, 255, 60},
        {206, 89, 12}, {242, 153, 56},
        {48, 85, 190}, {105, 150, 232},
        {16, 113, 32}, {36, 157, 52}, {64, 198, 80}, {105, 230, 117},
        {72, 105, 68}, {117, 153, 109},
        {133, 190, 16}, {182, 230, 60},
        {56, 72, 28}, {80, 97, 40}, {109, 125, 56},
    };
    memcpy(&vga_pal.colors[VGA_EXT_EXPANDED_COMMON_START], ext_expanded_common, sizeof(ext_expanded_common));

    // ========================================================================
    // Step 1: Build fixed reference set (192 colors, never merge)
    // 84 BK common (0xA0-0xF3) + 12 ext_common + 96 expanded_common
    // ========================================================================
    #define MAX_CUSTOM 4096
    typedef struct { oklab_color lab; int count; } custom_entry;

    oklab_color ref[192];
    int ref_count = 0;

    // BK common: 0xA0-0xF3 (84 entries)
    for(int i = 0xA0; i <= 0xF3; i++) {
        ref[ref_count++] = rgb_to_oklab(vga_pal.colors[i].r, vga_pal.colors[i].g, vga_pal.colors[i].b);
    }
    // Ext common (12)
    for(int i = 0; i < 12; i++) {
        ref[ref_count++] = rgb_to_oklab(ext_common[i].r, ext_common[i].g, ext_common[i].b);
    }
    // Expanded common (96)
    for(int i = 0; i < 96; i++) {
        ref[ref_count++] = rgb_to_oklab(ext_expanded_common[i].r, ext_expanded_common[i].g, ext_expanded_common[i].b);
    }
    if(vflag) fprintf(stderr, "Fixed reference: %d colors (84 BK common + 12 ext + 96 expanded)\n", ref_count);

    // ========================================================================
    // Step 2: Collect unique colors from opaque image pixels.
    // ========================================================================
    custom_entry *custom = malloc(MAX_CUSTOM * sizeof(custom_entry));
    int custom_count = 0;
    int transparent_pixels = 0;

    for(int i = 0; i < npixels; i++) {
        if(rgba[i*4+3] == 0) { transparent_pixels++; continue; }
        // Drop semi-transparent pixels that are magenta-tinted.
        // Magenta is (255,0,255). Check if pixel is close enough
        // that it's clearly a magenta blend, not a real color.
        if(rgba[i*4+3] < 255) {
            int dr = abs((int)rgba[i*4] - 255);
            int dg = (int)rgba[i*4+1];
            int db = abs((int)rgba[i*4+2] - 255);
            if(dr + dg + db < mag_thresh) { transparent_pixels++; continue; }
        }

        oklab_color lab = rgb_to_oklab(rgba[i*4], rgba[i*4+1], rgba[i*4+2]);
        int found = -1;
        for(int j = 0; j < custom_count; j++) {
            if(oklab_dist_sq(lab, custom[j].lab) < 0.0001) { found = j; break; }
        }
        if(found >= 0) {
            custom[found].count++;
        } else if(custom_count < MAX_CUSTOM) {
            custom[custom_count].lab = lab;
            custom[custom_count].count = 1;
            custom_count++;
        }
    }

    fprintf(stderr, "%d unique image colors, %d transparent pixels\n",
            custom_count, transparent_pixels);

    // ========================================================================
    // Step 3: Reduce to SLOT_SIZE (64) custom entries.
    //         Each round, pick the cheapest operation:
    //         - Cast: absorb an image color into its nearest fixed palette entry
    //         - Merge: combine two image colors into one (weighted average)
    //         Cost = distance / count — rare colors are cheap to cast/merge,
    //         common colors are expensive. Distance dominates so near-duplicates
    //         (like near-blacks) get merged first regardless of count.
    // ========================================================================
    int round = 0;
    while(custom_count > SLOT_SIZE) {
        // Find best cast (image color -> nearest fixed palette entry)
        // Cost = distance * midtone_weight * chroma_weight / log(count)
        // Midtone weight = 4*L*(1-L): peaks at 1.0 for L=0.5, near 0 for dark/bright.
        // Chroma weight = max(0.3, sqrt(a²+b²)): saturated colors (skin tones) are
        // expensive to cast, desaturated (grays) are cheap.
        double best_cast_cost = 1e9;
        int best_cast_i = -1;
        double best_cast_d = 0;
        for(int i = 0; i < custom_count; i++) {
            double d = 1e9;
            for(int j = 0; j < ref_count; j++) {
                double dd = oklab_dist_sq(custom[i].lab, ref[j]);
                if(dd < d) { d = dd; }
            }
            double L = custom[i].lab.L;
            double midtone = 4.0 * L * (1.0 - L);
            if(midtone < 0.01) midtone = 0.01;
            double chroma = sqrt(custom[i].lab.a * custom[i].lab.a + custom[i].lab.b * custom[i].lab.b);
            if(chroma < 0.3) chroma = 0.3;
            double cost = (d * midtone * chroma) / (custom[i].count > 1 ? log((double)custom[i].count) : 1.0);
            if(cost < best_cast_cost) { best_cast_cost = cost; best_cast_i = i; best_cast_d = d; }
        }

        // Find best merge (two image colors)
        // Same midtone + chroma bias: use average L and chroma of the two colors
        double best_merge_cost = 1e9;
        int best_merge_i = -1, best_merge_j = -1;
        double best_merge_d = 0;
        for(int i = 0; i < custom_count - 1; i++) {
            for(int j = i + 1; j < custom_count; j++) {
                double d = oklab_dist_sq(custom[i].lab, custom[j].lab);
                int total_count = custom[i].count + custom[j].count;
                double avg_L = (custom[i].lab.L * custom[i].count + custom[j].lab.L * custom[j].count) / total_count;
                double midtone = 4.0 * avg_L * (1.0 - avg_L);
                if(midtone < 0.01) midtone = 0.01;
                double avg_a = (custom[i].lab.a * custom[i].count + custom[j].lab.a * custom[j].count) / total_count;
                double avg_b = (custom[i].lab.b * custom[i].count + custom[j].lab.b * custom[j].count) / total_count;
                double chroma = sqrt(avg_a * avg_a + avg_b * avg_b);
                if(chroma < 0.3) chroma = 0.3;
                double cost = (d * midtone * chroma) / (total_count > 1 ? log((double)total_count) : 1.0);
                if(cost < best_merge_cost) { best_merge_cost = cost; best_merge_i = i; best_merge_j = j; best_merge_d = d; }
            }
        }

        if(best_cast_i < 0 && best_merge_i < 0) break;

        if(best_cast_cost <= best_merge_cost) {
            // Cast: remove this image color (it maps to a fixed entry)
            if(vflag && (round < 10 || custom_count <= SLOT_SIZE + 5)) {
                fprintf(stderr, "  Round %d: CAST dist²=%.4f count=%d cost=%.6f, %d custom remaining\n",
                        round, best_cast_d, custom[best_cast_i].count, best_cast_cost, custom_count - 1);
            }
            memmove(&custom[best_cast_i], &custom[best_cast_i + 1],
                    (custom_count - best_cast_i - 1) * sizeof(custom_entry));
            custom_count--;
        } else {
            // Merge: combine two image colors
            if(vflag && (round < 10 || custom_count <= SLOT_SIZE + 5)) {
                fprintf(stderr, "  Round %d: MERGE dist²=%.4f counts=%d+%d cost=%.6f, %d custom remaining\n",
                        round, best_merge_d, custom[best_merge_i].count, custom[best_merge_j].count, best_merge_cost, custom_count - 1);
            }
            int total = custom[best_merge_i].count + custom[best_merge_j].count;
            if(total > 0) {
                custom[best_merge_i].lab.L = (custom[best_merge_i].lab.L * custom[best_merge_i].count + custom[best_merge_j].lab.L * custom[best_merge_j].count) / total;
                custom[best_merge_i].lab.a = (custom[best_merge_i].lab.a * custom[best_merge_i].count + custom[best_merge_j].lab.a * custom[best_merge_j].count) / total;
                custom[best_merge_i].lab.b = (custom[best_merge_i].lab.b * custom[best_merge_i].count + custom[best_merge_j].lab.b * custom[best_merge_j].count) / total;
            }
            custom[best_merge_i].count = total;
            memmove(&custom[best_merge_j], &custom[best_merge_j + 1],
                    (custom_count - best_merge_j - 1) * sizeof(custom_entry));
            custom_count--;
        }
        round++;
    }

    fprintf(stderr, "%d custom colors after %d rounds (target: %d)\n",
            custom_count, round, SLOT_SIZE);

    // ========================================================================
    // Step 4: Build 256-entry output palette.
    //         84 BK common + 12 ext_common + 96 expanded_common + 64 custom = 256
    // ========================================================================
    vga_color out_pal[256];
    int oi = 0;

    // Expanded common (96)
    for(int i = 0; i < 96; i++) {
        out_pal[oi++] = ext_expanded_common[i];
    }
    // Custom slot (64)
    for(int i = 0; i < custom_count && i < SLOT_SIZE; i++) {
        uint8_t r, g, b;
        oklab_to_rgb(custom[i].lab, &r, &g, &b);
        out_pal[oi].r = r;
        out_pal[oi].g = g;
        out_pal[oi].b = b;
        oi++;
    }
    // BK common (84)
    for(int i = 0xA0; i <= 0xF3; i++) {
        out_pal[oi++] = vga_pal.colors[i];
    }
    // Ext common (12)
    for(int i = 0; i < 12; i++) {
        out_pal[oi++] = ext_common[i];
    }
    // Fill remaining with black
    while(oi < 256) {
        out_pal[oi].r = 0; out_pal[oi].g = 0; out_pal[oi].b = 0;
        oi++;
    }
    if(vflag) fprintf(stderr, "Output palette: 96 expanded + %d custom + 84 BK common + 12 ext = %d\n",
            custom_count, 96 + custom_count + 84 + 12);

    // ========================================================================
    // Step 5: Map every pixel to nearest output palette entry.
    // ========================================================================
    uint8_t *pixel_map = malloc(npixels);
    oklab_color out_lab[256];
    for(int i = 0; i < 256; i++) {
        out_lab[i] = rgb_to_oklab(out_pal[i].r, out_pal[i].g, out_pal[i].b);
    }

    for(int i = 0; i < npixels; i++) {
        if(rgba[i*4+3] == 0) {
            pixel_map[i] = 0;
            continue;
        }
        if(rgba[i*4+3] < 255) {
            int dr = abs((int)rgba[i*4] - 255);
            int dg = (int)rgba[i*4+1];
            int db = abs((int)rgba[i*4+2] - 255);
            if(dr + dg + db < mag_thresh) { pixel_map[i] = 0; continue; }
        }

        oklab_color lab = rgb_to_oklab(rgba[i*4], rgba[i*4+1], rgba[i*4+2]);
        double best_d = 1e9;
        int best_idx = 0;

        for(int j = 0; j < 256; j++) {
            double d = oklab_dist_sq(lab, out_lab[j]);
            if(d < best_d) { best_d = d; best_idx = j; }
        }

        pixel_map[i] = (uint8_t)best_idx;
    }

    // ========================================================================
    // Output paletted PNG
    // ========================================================================
    if(output->count) {
        FILE *fp = fopen(output->filename[0], "wb");
        if(!fp) {
            fprintf(stderr, "Cannot open %s for writing\n", output->filename[0]);
            goto cleanup;
        }

        png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        png_infop info = png_create_info_struct(png);
        if(setjmp(png_jmpbuf(png))) { fclose(fp); goto cleanup; }

        png_init_io(png, fp);
        png_set_IHDR(png, info, w, h, 8, PNG_COLOR_TYPE_PALETTE,
                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

        png_color png_pal[256];
        png_byte trans[256];
        memset(trans, 0xFF, sizeof(trans));
        trans[0] = 0; // index 0 is transparent
        for(int i = 0; i < 256; i++) {
            png_pal[i].red = out_pal[i].r;
            png_pal[i].green = out_pal[i].g;
            png_pal[i].blue = out_pal[i].b;
        }
        png_set_PLTE(png, info, png_pal, 256);
        png_set_tRNS(png, info, trans, 256, 0);

        png_write_info(png, info);
        for(int y = 0; y < h; y++) {
            png_write_row(png, pixel_map + y * w);
        }
        png_write_end(png, NULL);
        png_destroy_write_struct(&png, &info);
        fclose(fp);
        fprintf(stderr, "Wrote %dx%d paletted PNG to %s\n", w, h, output->filename[0]);
    }

    // Write slot palette binary (64 RGB triplets) for loading into game
    if(imgout->count) {
        FILE *fp = fopen(imgout->filename[0], "wb");
        if(fp) {
            uint8_t slot_rgb[SLOT_SIZE * 3];
            memset(slot_rgb, 0, sizeof(slot_rgb));
            // Custom colors are at indices 96..159 in output palette
            for(int i = 0; i < custom_count && i < SLOT_SIZE; i++) {
                slot_rgb[i*3]   = out_pal[96 + i].r;
                slot_rgb[i*3+1] = out_pal[96 + i].g;
                slot_rgb[i*3+2] = out_pal[96 + i].b;
            }
            fwrite(slot_rgb, 1, sizeof(slot_rgb), fp);
            fclose(fp);
            fprintf(stderr, "Wrote %d slot colors to %s\n", custom_count, imgout->filename[0]);
        }
    }

    cleanup:
    free(rgba);
    free(custom);
    free(pixel_map);

exit:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}