#include "video/surface.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include "utils/png_writer.h"
#include <stdlib.h>

// Each surface is tagged with a unique key. This is then used for texture atlas.
// This keeps track of the last index used.
static unsigned int guid = 0;

void surface_create(surface *sur, int w, int h) {
    sur->data = omf_calloc(1, w * h);
    sur->guid = guid++;
    sur->w = w;
    sur->h = h;
    sur->transparent = 0;
}

void surface_create_from_data(surface *sur, int w, int h, const unsigned char *src) {
    surface_create(sur, w, h);
    memcpy(sur->data, src, w * h);
}

void surface_create_from_data_flip(surface *sur, int w, int h, const unsigned char *src) {
    surface_create(sur, w, h);
    for(int y = 0; y < h; y++) {
        memcpy(sur->data + (h - y - 1) * w, src + y * w, w);
    }
}

void surface_create_from_vga(surface *sur, const sd_vga_image *src) {
    surface_create(sur, src->w, src->h);
    memcpy(sur->data, src->data, src->w * src->h);
    sur->transparent = -1;
}

void surface_create_from_surface(surface *sur, int w, int h, int src_x, int src_y, const surface *src) {
    surface_create(sur, w, h);
    surface_sub(sur, src, 0, 0, src_x, src_y, w, h, SUB_METHOD_NONE);
    sur->transparent = src->transparent;
}

void surface_create_from_image(surface *sur, image *img) {
    surface_create_from_data(sur, img->w, img->h, img->data);
    sur->transparent = -1;
}

int surface_to_image(const surface *sur, image *img) {
    img->w = sur->w;
    img->h = sur->h;
    img->data = sur->data;
    return 0;
}

void surface_set_transparency(surface *sur, int index) {
    sur->transparent = index;
}

void surface_free(surface *sur) {
    omf_free(sur->data);
}

void surface_clear(surface *sur) {
    memset(sur->data, 0, sur->w * sur->h);
    sur->guid = guid++;
}

void surface_create_from(surface *dst, const surface *src) {
    surface_create(dst, src->w, src->h);
    memcpy(dst->data, src->data, src->w * src->h);
    dst->transparent = src->transparent;
}

void surface_multiply_decal(surface *src, const surface *decal, int dst_x, int dst_y) {
    int src_offset, decal_offset;
    int color, value;
    for(int y = 0; y < decal->h; y++) {
        if((dst_y + y) >= src->h) {
            continue;
        }
        for(int x = 0; x < decal->w; x++) {
            if((dst_x + x) >= src->w) {
                continue;
            }
            src_offset = (dst_x + x + (dst_y + y) * src->w);
            decal_offset = (x + y * decal->w);
            if(src->data[src_offset] == 0) {
                continue;
            }
            if(decal->data[decal_offset] == 0) {
                continue;
            }
            color = src->data[src_offset] & 0xf0;
            value = src->data[src_offset] & 0x0f;
            value = (value * decal->data[decal_offset]) >> 4;
            if(value > 15) {
                value = 15;
            }
            src->data[src_offset] = color | value;
        }
    }
    src->guid = guid++;
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
        }
    }
    dst->guid = guid++;
}

static uint8_t find_closest_gray(const vga_palette *pal, int range_start, int range_end, int ref) {
    uint8_t closest = 0, current;
    int closest_dist = 256, dist;

    for(int i = range_start; i <= range_end; i++) {
        current = pal->colors[i].r; // Grayscale, r = g = b so pick any.
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

void surface_flatten_to_mask(surface *sur, uint8_t value) {
    uint8_t idx;
    for(int i = 0; i < sur->w * sur->h; i++) {
        idx = sur->data[i];
        if(idx == sur->transparent)
            continue;
        sur->data[i] = value;
    }
    sur->guid = guid++;
}

void surface_convert_to_grayscale(surface *sur, const vga_palette *pal, int range_start, int range_end,
                                  int ignore_below) {
    float r, g, b;
    uint8_t idx;
    unsigned char mapping[256];

    // Make a mapping for fast search.
    for(int i = 0; i < 256; i++) {
        if(i < ignore_below) {
            mapping[i] = i;
            continue;
        }
        r = pal->colors[i].r * 0.3;
        g = pal->colors[i].g * 0.59;
        b = pal->colors[i].b * 0.11;
        mapping[i] = find_closest_gray(pal, range_start, range_end, r + g + b);
    }

    // Convert the image using the mapping
    for(int i = 0; i < sur->w * sur->h; i++) {
        idx = sur->data[i];
        if(idx == sur->transparent)
            continue;
        sur->data[i] = mapping[idx];
    }
    sur->guid = guid++;
}

void surface_convert_har_to_grayscale(surface *sur, uint8_t brightness) {
    uint8_t idx;
    for(int i = 0; i < sur->w * sur->h; i++) {
        idx = sur->data[i];
        if(idx != sur->transparent && idx < 0x60) {
            sur->data[i] = 0xD0 + brightness * (idx % 0x10) / 0x0F;
        }
    }
    sur->guid = guid++;
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
    sur->guid = guid++;
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
    sur->guid = guid++;
}

bool surface_write_png(const surface *sur, const vga_palette *pal, const char *filename) {
    return write_paletted_png(filename, sur->w, sur->h, pal, sur->data);
}
