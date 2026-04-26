/** @file main.c
 * @brief Palette optimization tool — pack custom colors into portrait or scene palettes.
 *
 * Portrait mode (default):
 * 1. Build fixed reference set: base non-slot (192) + ext_common (12) + expanded_common (96) = 300 colors
 * 2. Map each opaque image pixel to nearest reference color. If close enough, it's "matched".
 * 3. Collect unmatched image pixels as "custom" candidates.
 * 4. Collapse custom candidates down to 64 by iteratively merging closest pair (oklab distance).
 * 5. Output: 256-entry palette = 96 expanded_common + 64 custom + 84 BK common + 12 ext_common.
 *
 * Scene mode (--scene):
 * 1. Load BK palette as starting point.
 * 2. Identify locked indices (HAR zone 0x01-0x5F, menu colors 0xFA-0xFF) vs free indices.
 * 3. Collect unique colors from all input sprite PNGs.
 * 4. Collapse to free index budget using same algorithm as portrait mode.
 * 5. Assign collapsed colors to free indices, output full 256-color palette.
 */

#include "video/vga_common_colors.h"
#include "utils/oklab.h"
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
    struct arg_file *input = arg_filen("i", "input", "<png>", 1, 256, "Input PNG image (multiple for --scene)");
    struct arg_file *bkfile = arg_file1("b", "bk", "<bk>", "BK file to load palette from");
    struct arg_file *output = arg_file0("o", "output", "<png>", "Output paletted PNG");
    struct arg_file *imgout = arg_file0("p", "palette", "<bin>", "Output palette binary");
    struct arg_int *slot = arg_int0("s", "slot", "<int>", "Portrait slot index (0-4, default 0)");
    struct arg_str *scene_type = arg_str0(NULL, "scene", "<type>", "Scene palette mode: arena, vs, melee, mechlab, tournament, other");
    struct arg_file *sprite_out = arg_file0(NULL, "sprite-out", "<dir>", "Output directory for per-sprite paletted PNGs (scene mode only)");
    struct arg_dbl *magthresh = arg_dbl0("m", "magenta-thresh", "<int>", "Manhattan distance threshold for magenta key color (default 350)");
    struct arg_lit *verbose = arg_lit0("v", "verbose", "Print detailed progress");
    struct arg_end *end = arg_end(20);
    void *argtable[] = {help, input, bkfile, output, imgout, slot, scene_type, sprite_out, magthresh, verbose, end};

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

    // Read all input images and collect unique colors
    #define MAX_CUSTOM 4096
    typedef struct { oklab_color lab; int count; } custom_entry;

    uint8_t *rgba = NULL;
    int w, h;
    int total_npixels = 0;
    int transparent_pixels = 0;
    custom_entry *custom = malloc(MAX_CUSTOM * sizeof(custom_entry));
    int custom_count = 0;
    int file_count = input->count;

    for(int f = 0; f < file_count; f++) {
        if(read_png(input->filename[f], &rgba, &w, &h) != 0) {
            fprintf(stderr, "Failed to read %s\n", input->filename[f]);
            goto cleanup;
        }
        int npixels = w * h;
        total_npixels += npixels;
        if(vflag) fprintf(stderr, "Image %d: %s %dx%d (%d pixels)\n", f + 1, input->filename[f], w, h, npixels);

        for(int i = 0; i < npixels; i++) {
            if(rgba[i*4+3] == 0) { transparent_pixels++; continue; }
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
        free(rgba);
        rgba = NULL;
    }
    if(vflag) fprintf(stderr, "Total: %d pixels across %d files\n", total_npixels, file_count);

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

    // Extended and expanded common colors from shared definition
    memcpy(&vga_pal.colors[VGA_EXT_COMMON_START], vga_ext_common, sizeof(vga_ext_common));
    memcpy(&vga_pal.colors[VGA_EXT_EXPANDED_COMMON_START], vga_ext_expanded_common, sizeof(vga_ext_expanded_common));

    // ========================================================================
    // Determine mode: portrait (default) or scene
    // ========================================================================
    int is_scene = (scene_type->count > 0);

    // For scene mode, determine which indices are locked vs free
    int locked[256] = {0}; // 1 = locked, 0 = free
    int free_count = 0;

    if(is_scene) {
        // Start with ALL indices locked, then unlock only genuinely free indices.
        // Per Andrew: almost nothing is locked. HAR indices remap to 1024-space
        // at runtime. The ONLY locked zones are:
        // - 0x00: always transparent
        // - 0xA0-0xF3: portrait common (only for portrait-showing scenes)
        // - 0xF4-0xFF: fixed extended common colors (remapped at runtime but
        //   palette values don't change)
        for(int i = 0; i < 256; i++) locked[i] = 1;

        const char *stype = scene_type->sval[0];
        if(strcmp(stype, "arena") == 0 || strcmp(stype, "vs") == 0) {
            // Arena/VS: 2 HARs + 2 portraits. Lock portrait common + ext common.
            // Free: 0x01-0x9F. ~159 indices.
            for(int i = 0x01; i <= 0x9F; i++) locked[i] = 0;
            if(strcmp(stype, "vs") == 0) {
                locked[0xF6] = 1;
                locked[0xFB] = 1;
                locked[0xFC] = 1;
            }
        } else if(strcmp(stype, "melee") == 0) {
            // MELEE: 2 HARs, no portraits. Lock ext common only.
            // Free: 0x01-0xF3. ~243 indices.
            for(int i = 0x01; i <= 0xF3; i++) locked[i] = 0;
            locked[0xF6] = 1;
            locked[0xFB] = 1;
        } else if(strcmp(stype, "mechlab") == 0) {
            // MECHLAB: 1 HAR + portraits. Lock portrait common + ext common.
            // Free: 0x01-0x9F. ~159 indices.
            for(int i = 0x01; i <= 0x9F; i++) locked[i] = 0;
        } else if(strcmp(stype, "tournament") == 0) {
            // Tournament cutscenes: 1 HAR double. Lock ext common only.
            // Free: 0x01-0xF3. ~243 indices.
            for(int i = 0x01; i <= 0xF3; i++) locked[i] = 0;
        } else if(strcmp(stype, "other") == 0) {
            // Other scenes: no HARs, no portraits. Lock ext common only.
            // Free: 0x01-0xF3. ~243 indices.
            for(int i = 0x01; i <= 0xF3; i++) locked[i] = 0;
        } else {
            fprintf(stderr, "Unknown scene type: %s (valid: arena, vs, melee, mechlab, tournament, other)\n", stype);
            goto exit;
        }

        // Count free indices
        for(int i = 0; i < 256; i++) {
            if(!locked[i]) free_count++;
        }
        if(vflag) fprintf(stderr, "Scene mode: %s, %d free indices, %d locked\n", stype, free_count, 256 - free_count);
    }

    // ========================================================================
    // Step 1: Build fixed reference set
    // Portrait mode: 192 colors (84 BK common + 12 ext + 96 expanded)
    // Scene mode: locked palette indices
    // ========================================================================

    oklab_color ref[256];
    int ref_count = 0;

    if(is_scene) {
        // Scene mode: reference set = all locked palette indices
        for(int i = 0; i < 256; i++) {
            if(locked[i]) {
                ref[ref_count++] = rgb_to_oklab(vga_pal.colors[i].r, vga_pal.colors[i].g, vga_pal.colors[i].b);
            }
        }
        if(vflag) fprintf(stderr, "Fixed reference: %d locked palette colors\n", ref_count);
    } else {
        // Portrait mode: 84 BK common (0xA0-0xF3) + 12 ext_common + 96 expanded_common
        for(int i = 0xA0; i <= 0xF3; i++) {
            ref[ref_count++] = rgb_to_oklab(vga_pal.colors[i].r, vga_pal.colors[i].g, vga_pal.colors[i].b);
        }
        for(int i = 0; i < 12; i++) {
            ref[ref_count++] = rgb_to_oklab(vga_ext_common[i].r, vga_ext_common[i].g, vga_ext_common[i].b);
        }
        for(int i = 0; i < 96; i++) {
            ref[ref_count++] = rgb_to_oklab(vga_ext_expanded_common[i].r, vga_ext_expanded_common[i].g, vga_ext_expanded_common[i].b);
        }
        if(vflag) fprintf(stderr, "Fixed reference: %d colors (84 BK common + 12 ext + 96 expanded)\n", ref_count);
    }

    int target_count = is_scene ? free_count : SLOT_SIZE;
    fprintf(stderr, "%d unique image colors, %d transparent pixels\n",
            custom_count, transparent_pixels);

    // ========================================================================
    // Step 3: Reduce to target_count custom entries.
    //         Each round, pick the cheapest operation:
    //         - Cast: absorb an image color into its nearest fixed palette entry
    //         - Merge: combine two image colors into one (weighted average)
    //         Cost = distance / count — rare colors are cheap to cast/merge,
    //         common colors are expensive. Distance dominates so near-duplicates
    //         (like near-blacks) get merged first regardless of count.
    // ========================================================================
    int round = 0;
    while(custom_count > target_count) {
        // Find best cast (image color -> nearest fixed palette entry)
        // Cost = distance * midtone_weight * chroma_weight / log(count)
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
            if(vflag && (round < 10 || custom_count <= target_count + 5)) {
                fprintf(stderr, "  Round %d: CAST dist²=%.4f count=%d cost=%.6f, %d custom remaining\n",
                        round, best_cast_d, custom[best_cast_i].count, best_cast_cost, custom_count - 1);
            }
            memmove(&custom[best_cast_i], &custom[best_cast_i + 1],
                    (custom_count - best_cast_i - 1) * sizeof(custom_entry));
            custom_count--;
        } else {
            if(vflag && (round < 10 || custom_count <= target_count + 5)) {
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
            custom_count, round, target_count);

    // ========================================================================
    // Step 4: Build 256-entry output palette.
    // Portrait mode: 96 expanded + 64 custom + 84 BK common + 12 ext = 256
    // Scene mode: start from BK palette, overwrite free indices with custom colors
    // ========================================================================
    vga_color out_pal[256];

    if(is_scene) {
        // Start from BK palette
        memcpy(out_pal, vga_pal.colors, 256 * sizeof(vga_color));

        // Overwrite free indices with collapsed custom colors
        int ci = 0;
        for(int i = 0; i < 256 && ci < custom_count; i++) {
            if(!locked[i]) {
                uint8_t r, g, b;
                oklab_to_rgb(custom[ci].lab, &r, &g, &b);
                out_pal[i].r = r;
                out_pal[i].g = g;
                out_pal[i].b = b;
                ci++;
            }
        }
        if(vflag) fprintf(stderr, "Scene palette: %d custom colors assigned to free indices\n", ci);
    } else {
        // Portrait mode: 96 expanded + 64 custom + 84 BK common + 12 ext
        int oi = 0;
        for(int i = 0; i < 96; i++) {
            out_pal[oi++] = vga_ext_expanded_common[i];
        }
        for(int i = 0; i < custom_count && i < SLOT_SIZE; i++) {
            uint8_t r, g, b;
            oklab_to_rgb(custom[i].lab, &r, &g, &b);
            out_pal[oi].r = r;
            out_pal[oi].g = g;
            out_pal[oi].b = b;
            oi++;
        }
        for(int i = 0xA0; i <= 0xF3; i++) {
            out_pal[oi++] = vga_pal.colors[i];
        }
        for(int i = 0; i < 12; i++) {
            out_pal[oi++] = vga_ext_common[i];
        }
        while(oi < 256) {
            out_pal[oi].r = 0; out_pal[oi].g = 0; out_pal[oi].b = 0;
            oi++;
        }
        if(vflag) fprintf(stderr, "Output palette: 96 expanded + %d custom + 84 BK common + 12 ext = %d\n",
                custom_count, 96 + custom_count + 84 + 12);
    }

    // ========================================================================
    // Step 5: Output
    // ========================================================================

    if(is_scene) {
        // Scene mode: output full 256-color BK palette
        if(imgout->count) {
            FILE *fp = fopen(imgout->filename[0], "wb");
            if(fp) {
                uint8_t pal_rgb[256 * 3];
                for(int i = 0; i < 256; i++) {
                    pal_rgb[i*3]   = out_pal[i].r;
                    pal_rgb[i*3+1] = out_pal[i].g;
                    pal_rgb[i*3+2] = out_pal[i].b;
                }
                fwrite(pal_rgb, 1, sizeof(pal_rgb), fp);
                fclose(fp);
                fprintf(stderr, "Wrote 256-color BK palette to %s\n", imgout->filename[0]);
            }
        }

        // Write text format palette for inspection
        if(output->count) {
            FILE *fp = fopen(output->filename[0], "w");
            if(fp) {
                for(int i = 0; i < 256; i++) {
                    fprintf(fp, "%3d: %3d %3d %3d\n", i, out_pal[i].r, out_pal[i].g, out_pal[i].b);
                }
                fclose(fp);
                fprintf(stderr, "Wrote text palette to %s\n", output->filename[0]);
            }
        }

        // Per-sprite paletted PNG output for verification
        if(sprite_out->count) {
            oklab_color out_lab[256];
            for(int i = 0; i < 256; i++) {
                out_lab[i] = rgb_to_oklab(out_pal[i].r, out_pal[i].g, out_pal[i].b);
            }

            for(int f = 0; f < file_count; f++) {
                uint8_t *sprite_rgba = NULL;
                int sw, sh;
                if(read_png(input->filename[f], &sprite_rgba, &sw, &sh) != 0) {
                    fprintf(stderr, "Warning: failed to re-read %s for sprite output\n", input->filename[f]);
                    continue;
                }

                // Build output filename: <sprite-out-dir>/<basename>_pal.png
                const char *in_name = input->filename[f];
                const char *base = strrchr(in_name, '/');
                base = base ? base + 1 : in_name;
                char out_path[1024];
                const char *dir = sprite_out->filename[0];
                snprintf(out_path, sizeof(out_path), "%s/%s", dir, base);
                // Replace .png suffix with _pal.png
                char *dot = strrchr(out_path, '.');
                if(dot && strcmp(dot, ".png") == 0) {
                    memmove(dot + 4, dot + 4, strlen(dot + 4) + 1);
                    memcpy(dot, "_pal.png", 8);
                }
                // Collapse double slashes from trailing / in dir
                char *slash = strstr(out_path, "//");
                if(slash) {
                    memmove(slash, slash + 1, strlen(slash + 1) + 1);
                }

                int snpixels = sw * sh;
                uint8_t *sprite_map = malloc(snpixels);
                for(int i = 0; i < snpixels; i++) {
                    if(sprite_rgba[i*4+3] == 0) {
                        sprite_map[i] = 0;
                        continue;
                    }
                    if(sprite_rgba[i*4+3] < 255) {
                        int dr = abs((int)sprite_rgba[i*4] - 255);
                        int dg = (int)sprite_rgba[i*4+1];
                        int db = abs((int)sprite_rgba[i*4+2] - 255);
                        if(dr + dg + db < mag_thresh) { sprite_map[i] = 0; continue; }
                    }
                    oklab_color lab = rgb_to_oklab(sprite_rgba[i*4], sprite_rgba[i*4+1], sprite_rgba[i*4+2]);
                    double best_d = 1e9;
                    int best_idx = 0;
                    for(int j = 0; j < 256; j++) {
                        double d = oklab_dist_sq(lab, out_lab[j]);
                        if(d < best_d) { best_d = d; best_idx = j; }
                    }
                    sprite_map[i] = (uint8_t)best_idx;
                }

                FILE *fp = fopen(out_path, "wb");
                if(fp) {
                    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
                    png_infop info = png_create_info_struct(png);
                    if(!setjmp(png_jmpbuf(png))) {
                        png_init_io(png, fp);
                        png_set_IHDR(png, info, sw, sh, 8, PNG_COLOR_TYPE_PALETTE,
                                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
                        png_color png_pal[256];
                        png_byte trans[256];
                        memset(trans, 0xFF, sizeof(trans));
                        trans[0] = 0;
                        for(int i = 0; i < 256; i++) {
                            png_pal[i].red = out_pal[i].r;
                            png_pal[i].green = out_pal[i].g;
                            png_pal[i].blue = out_pal[i].b;
                        }
                        png_set_PLTE(png, info, png_pal, 256);
                        png_set_tRNS(png, info, trans, 256, 0);
                        png_write_info(png, info);
                        for(int y = 0; y < sh; y++) {
                            png_write_row(png, sprite_map + y * sw);
                        }
                        png_write_end(png, NULL);
                        png_destroy_write_struct(&png, &info);
                        fprintf(stderr, "Wrote sprite %dx%d paletted PNG to %s\n", sw, sh, out_path);
                    } else {
                        png_destroy_write_struct(&png, &info);
                    }
                    fclose(fp);
                }
                free(sprite_rgba);
                free(sprite_map);
            }
        }
    } else {
        // Portrait mode: map pixels and output paletted PNG (single file)
        uint8_t *rgba_out = NULL;
        int w_out, h_out;
        if(read_png(input->filename[0], &rgba_out, &w_out, &h_out) != 0) {
            fprintf(stderr, "Failed to re-read %s for output\n", input->filename[0]);
            goto cleanup;
        }
        int npixels = w_out * h_out;

        uint8_t *pixel_map = malloc(npixels);
        oklab_color out_lab[256];
        for(int i = 0; i < 256; i++) {
            out_lab[i] = rgb_to_oklab(out_pal[i].r, out_pal[i].g, out_pal[i].b);
        }

        for(int i = 0; i < npixels; i++) {
            if(rgba_out[i*4+3] == 0) {
                pixel_map[i] = 0;
                continue;
            }
            if(rgba_out[i*4+3] < 255) {
                int dr = abs((int)rgba_out[i*4] - 255);
                int dg = (int)rgba_out[i*4+1];
                int db = abs((int)rgba_out[i*4+2] - 255);
                if(dr + dg + db < mag_thresh) { pixel_map[i] = 0; continue; }
            }

            oklab_color lab = rgb_to_oklab(rgba_out[i*4], rgba_out[i*4+1], rgba_out[i*4+2]);
            double best_d = 1e9;
            int best_idx = 0;

            for(int j = 0; j < 256; j++) {
                double d = oklab_dist_sq(lab, out_lab[j]);
                if(d < best_d) { best_d = d; best_idx = j; }
            }

            pixel_map[i] = (uint8_t)best_idx;
        }

        if(output->count) {
            FILE *fp = fopen(output->filename[0], "wb");
            if(!fp) {
                fprintf(stderr, "Cannot open %s for writing\n", output->filename[0]);
                free(rgba_out);
                free(pixel_map);
                goto cleanup;
            }

            png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            png_infop info = png_create_info_struct(png);
            if(setjmp(png_jmpbuf(png))) { fclose(fp); free(rgba_out); free(pixel_map); goto cleanup; }

            png_init_io(png, fp);
            png_set_IHDR(png, info, w_out, h_out, 8, PNG_COLOR_TYPE_PALETTE,
                         PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

            png_color png_pal[256];
            png_byte trans[256];
            memset(trans, 0xFF, sizeof(trans));
            trans[0] = 0;
            for(int i = 0; i < 256; i++) {
                png_pal[i].red = out_pal[i].r;
                png_pal[i].green = out_pal[i].g;
                png_pal[i].blue = out_pal[i].b;
            }
            png_set_PLTE(png, info, png_pal, 256);
            png_set_tRNS(png, info, trans, 256, 0);

            png_write_info(png, info);
            for(int y = 0; y < h_out; y++) {
                png_write_row(png, pixel_map + y * w_out);
            }
            png_write_end(png, NULL);
            png_destroy_write_struct(&png, &info);
            fclose(fp);
            fprintf(stderr, "Wrote %dx%d paletted PNG to %s\n", w_out, h_out, output->filename[0]);
        }

        free(rgba_out);
        free(pixel_map);
    }

    cleanup:
    free(custom);

exit:
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}