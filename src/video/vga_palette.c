#include "video/vga_palette.h"
#include "utils/miscmath.h"
#include <assert.h>
#include <string.h>

void vga_palette_init(vga_palette *palette) {
    assert(palette != NULL);
    memset(palette, 0, sizeof(vga_palette));
}

void vga_palette_tint_range(vga_palette *pal, vga_index ref_index, vga_index start, vga_index end, uint8_t step) {
    vga_color ref = pal->colors[ref_index];
    uint32_t m, u;
    for(vga_index i = start; i < end; i++) {
        m = max3(pal->colors[i].r, pal->colors[i].g, pal->colors[i].b);
        u = (step * m) >> 8;
        pal->colors[i].r += u * (ref.r - pal->colors[i].r) >> 8;
        pal->colors[i].g += u * (ref.g - pal->colors[i].g) >> 8;
        pal->colors[i].b += u * (ref.b - pal->colors[i].b) >> 8;
    }
}

void vga_palette_mix_range(vga_palette *pal, vga_index ref_index, vga_index start, vga_index end, uint8_t step) {
    vga_color ref = pal->colors[ref_index];
    uint32_t inv = 255 - step;
    for(vga_index i = start; i < end; i++) {
        pal->colors[i].r = (pal->colors[i].r * inv + ref.r * step) >> 8;
        pal->colors[i].g = (pal->colors[i].g * inv + ref.g * step) >> 8;
        pal->colors[i].b = (pal->colors[i].b * inv + ref.b * step) >> 8;
    }
}
