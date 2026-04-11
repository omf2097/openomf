#include "video/vga_palette.h"
#include "utils/miscmath.h"
#include <assert.h>
#include <string.h>

void vga_palette_init(vga_palette *palette) {
    assert(palette != NULL);
    memset(palette, 0, sizeof(vga_palette));
}

void vga_palette_tint_range(vga_palette *pal, vga_index ref_index, vga_index start, vga_index end, uint8_t step) {
    assert(start >= 0 && end < VGA_PALETTE_SIZE && end >= start);
    const vga_color ref = pal->colors[ref_index];
    for(vga_index i = start; i < end; i++) {
        const uint32_t m = max3(pal->colors[i].r, pal->colors[i].g, pal->colors[i].b);
        const uint32_t u = (step * m) >> 8;
        pal->colors[i].r += u * (ref.r - pal->colors[i].r) >> 8;
        pal->colors[i].g += u * (ref.g - pal->colors[i].g) >> 8;
        pal->colors[i].b += u * (ref.b - pal->colors[i].b) >> 8;
    }
}

void vga_palette_mix_range(vga_palette *pal, vga_index ref_index, vga_index start, vga_index end, uint8_t step) {
    assert(start >= 0 && end < VGA_PALETTE_SIZE && end >= start);
    const vga_color ref = pal->colors[ref_index];
    const uint32_t inv = 255 - step;
    for(vga_index i = start; i < end; i++) {
        pal->colors[i].r = (pal->colors[i].r * inv + ref.r * step) >> 8;
        pal->colors[i].g = (pal->colors[i].g * inv + ref.g * step) >> 8;
        pal->colors[i].b = (pal->colors[i].b * inv + ref.b * step) >> 8;
    }
}

void vga_palette_darken(vga_palette *pal, uint8_t step) {
    const uint32_t inv = 255 - step;
    for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
        pal->colors[i].r = (pal->colors[i].r * inv) >> 8;
        pal->colors[i].g = (pal->colors[i].g * inv) >> 8;
        pal->colors[i].b = (pal->colors[i].b * inv) >> 8;
    }
}

void vga_palette_light_range(vga_palette *pal, uint8_t gray, vga_index start, vga_index end, int32_t blend_factor) {
    assert(start >= 0 && end < VGA_PALETTE_SIZE && end >= start);
    const int32_t gr = gray; // widen to prevent underflow
    vga_color *colors = pal->colors;
    for(vga_index i = start; i < end; i++) {
        const int rgb_max = max3(colors[i].r, colors[i].g, colors[i].b);
        colors[i].r = clamp(colors[i].r + (gr - colors[i].r) * blend_factor / 256 * rgb_max / 256, 0, 255);
        colors[i].g = clamp(colors[i].g + (gr - colors[i].g) * blend_factor / 256 * rgb_max / 256, 0, 255);
        colors[i].b = clamp(colors[i].b + (gr - colors[i].b) * blend_factor / 256 * rgb_max / 256, 0, 255);
    }
}
