#include "video/screen_surface.h"
#include "utils/allocator.h"
#include "video/surface.h"
#include <string.h>

void screen_surface_create(screen_surface *sur, const int w, const int h) {
    sur->data = omf_calloc(w * h, sizeof(vga_index));
    sur->w = w;
    sur->h = h;
}

void screen_surface_create_from(screen_surface *dst, const screen_surface *src) {
    screen_surface_create(dst, src->w, src->h);
    memcpy(dst->data, src->data, src->w * src->h * sizeof(vga_index));
}

void screen_surface_create_from_u16_flip(screen_surface *sur, const int w, const int h, const uint16_t *src,
                                         const float scale) {
    screen_surface_create(sur, w, h);
    for(int y = 0; y < h; y++) {
        const uint16_t *src_row = src + y * w;
        vga_index *dst_row = sur->data + (h - y - 1) * w;
        for(int x = 0; x < w; x++) {
            dst_row[x] = (vga_index)(src_row[x] * scale + 0.5f);
        }
    }
}

void screen_surface_free(screen_surface *sur) {
    omf_free(sur->data);
}

static uint8_t find_closest_gray(const vga_palette *pal, const vga_index range_start, const vga_index range_end,
                                 const int ref) {
    uint8_t closest = 0;
    int closest_dist = 256;

    for(int i = range_start; i <= range_end; i++) {
        const uint8_t current = pal->colors[i].r;
        int dist = current - ref;
        if(dist < 0) {
            dist = -dist;
        }
        if(dist > closest_dist) {
            break;
        }
        if(dist < closest_dist) {
            closest_dist = dist;
            closest = i;
        }
    }

    return closest;
}

void screen_surface_to_grayscale(const screen_surface *src, surface *dst, const vga_palette *pal,
                                 const vga_index range_start, const vga_index range_end, const int ignore_below) {
    unsigned char mapping[VGA_PALETTE_SIZE];

    for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
        if(i < ignore_below) {
            mapping[i] = i;
            continue;
        }
        const float r = pal->colors[i].r * 0.3;
        const float g = pal->colors[i].g * 0.59;
        const float b = pal->colors[i].b * 0.11;
        mapping[i] = find_closest_gray(pal, range_start, range_end, r + g + b);
    }

    surface_create(dst, src->w, src->h);
    for(int i = 0; i < src->w * src->h; i++) {
        const vga_index idx = src->data[i];
        if(idx >= 0 && idx < VGA_PALETTE_SIZE) {
            dst->data[i] = mapping[idx];
        }
    }
    surface_set_transparency(dst, -1);
}
