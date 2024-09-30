#include "video/surface.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include <png.h>
#include <stdlib.h>
#include <utils/log.h>

#define FNV_INITIAL ((uint32_t)2166136261)

static void fnv_32a_buf(uint32_t *hash, const void *buf, unsigned int len) {
    unsigned char *bp = (unsigned char *)buf;
    unsigned char *be = bp + len;
    while(bp < be) {
        *hash ^= (uint32_t)*bp++;
        *hash *= ((uint32_t)0x01000193);
    }
}

static inline void create_hash(surface *sur) {
    sur->hash = FNV_INITIAL;
    fnv_32a_buf(&sur->hash, sur->data, sur->w * sur->h);
    fnv_32a_buf(&sur->hash, sur->stencil, sur->w * sur->h);
}

void surface_create(surface *sur, int w, int h) {
    sur->data = omf_calloc(1, w * h);
    sur->stencil = omf_calloc(1, w * h);
    sur->hash = 0;
    sur->w = w;
    sur->h = h;
}

void surface_create_from_data(surface *sur, int w, int h, const unsigned char *src) {
    surface_create(sur, w, h);
    memcpy(sur->data, src, w * h);
    memset(sur->stencil, 1, w * h);
    create_hash(sur);
}

void surface_create_from_data_flip(surface *sur, int w, int h, const unsigned char *src) {
    surface_create(sur, w, h);
    for(int y = 0; y < h; y++) {
        memcpy(sur->data + (h - y - 1) * w, src + y * w, w);
    }
    memset(sur->stencil, 1, w * h);
    create_hash(sur);
}

void surface_create_from_vga(surface *sur, const sd_vga_image *src) {
    surface_create(sur, src->w, src->h);
    memcpy(sur->data, src->data, src->w * src->h);
    memcpy(sur->stencil, src->stencil, src->w * src->h);
    create_hash(sur);
}

void surface_create_from_surface(surface *sur, int w, int h, int src_x, int src_y, const surface *src) {
    surface_create(sur, w, h);
    surface_sub(sur, src, 0, 0, src_x, src_y, w, h, SUB_METHOD_NONE);
    create_hash(sur);
}

void surface_generate_stencil(const surface *sur, int index) {
    for(int i = 0; i < sur->w * sur->h; i++) {
        sur->stencil[i] = (sur->data[i] == index) ? 0 : 1;
    }
}

void surface_create_from_image(surface *sur, image *img) {
    surface_create_from_data(sur, img->w, img->h, img->data);
}

int surface_to_image(surface *sur, image *img) {
    img->w = sur->w;
    img->h = sur->h;
    img->data = sur->data;
    return 0;
}

void surface_free(surface *sur) {
    omf_free(sur->data);
    omf_free(sur->stencil);
}

void surface_clear(surface *sur) {
    memset(sur->data, 0, sur->w * sur->h);
    create_hash(sur);
}

void surface_create_from(surface *dst, const surface *src) {
    surface_create(dst, src->w, src->h);
    memcpy(dst->data, src->data, src->w * src->h);
    memcpy(dst->stencil, src->stencil, src->w * src->h);
    create_hash(dst);
}

// Copies a an area of old surface to an entirely new surface
void surface_sub(surface *dst, const surface *src, int dst_x, int dst_y, int src_x, int src_y, int w, int h,
                 int method) {
    int src_offset, dst_offset;
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < w; x++) {
            src_offset = (src_x + x + (src_y + y) * src->w);
            switch(method) {
                case SUB_METHOD_MIRROR:
                    dst_offset = (dst_x + (w - x - 1) + (dst_y + y) * dst->w);
                    break;
                default:
                    dst_offset = (dst_x + x + (dst_y + y) * dst->w);
                    break;
            }
            dst->data[dst_offset] = src->data[src_offset];
            dst->stencil[dst_offset] = src->stencil[src_offset];
        }
    }
    create_hash(dst);
}

static uint8_t find_closest_gray(const screen_palette *pal, int range_start, int range_end, int ref) {
    uint8_t closest = 0, current;
    int closest_dist = 256, dist;

    for(int i = range_start; i <= range_end; i++) {
        current = pal->data[i][0]; // Grayscale, r = g = b so pick any.
        dist = current - ref;
        if(dist < 0) {
            dist = -dist;
        }
        if(dist > closest_dist) {
            // We passed the optimum point, stop.
            break;
        }
        if(dist < closest_dist) {
            closest_dist = dist;
            closest = i;
        }
    }

    return closest;
}

void surface_convert_to_grayscale(surface *sur, const screen_palette *pal, int range_start, int range_end) {
    float r, g, b;
    uint8_t idx;
    unsigned char mapping[256];

    // Make a mapping for fast search.
    for(int i = 0; i < 256; i++) {
        r = pal->data[i][0] * 0.3;
        g = pal->data[i][1] * 0.59;
        b = pal->data[i][2] * 0.11;
        mapping[i] = find_closest_gray(pal, range_start, range_end, r + g + b);
    }

    // Convert the image using the mapping
    for(int i = 0; i < sur->w * sur->h; i++) {
        idx = sur->data[i];
        sur->data[i] = mapping[idx];
    }
    create_hash(sur);
}

void surface_compress_index_blocks(surface *sur, int range_start, int range_end, int block_size, int amount) {
    uint8_t idx, real_start, old_idx, new_idx;
    for(int i = 0; i < sur->w * sur->h; i++) {
        idx = sur->data[i];
        if(idx >= range_start && idx < range_end) {
            real_start = idx - range_start;
            old_idx = real_start % block_size;
            new_idx = max2(0, old_idx - amount);
            sur->data[i] = idx - old_idx + new_idx;
        }
    }
    create_hash(sur);
}

void surface_compress_remap(surface *sur, int range_start, int range_end, int remap_to, int amount) {
    uint8_t idx, real_start, d;
    for(int i = 0; i < sur->w * sur->h; i++) {
        idx = sur->data[i];
        if(idx >= range_start && idx < range_end) {
            real_start = idx - range_start;
            if(real_start - amount < range_start) {
                d = abs(real_start - amount);
                sur->data[i] = remap_to - d;
            }
        }
    }
    create_hash(sur);
}

bool surface_write_png(const surface *sur, const screen_palette *pal, const char *filename) {
    png_image out;
    memset(&out, 0, sizeof(out));
    out.version = PNG_IMAGE_VERSION;
    out.opaque = NULL;
    out.width = sur->w;
    out.height = sur->h;
    out.flags = 0;
    out.format = PNG_FORMAT_RGB_COLORMAP;
    out.colormap_entries = 256;
    png_image_write_to_file(&out, filename, 0, sur->data, sur->w, pal->data);
    if(PNG_IMAGE_FAILED(out)) {
        PERROR("Unable to write PNG file: %s", out.message);
        return false;
    }
    return true;
}
