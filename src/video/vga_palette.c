#include "video/vga_palette.h"
#include "utils/miscmath.h"
#include <assert.h>
#include <string.h>

void vga_palette_init(vga_palette *palette) {
    assert(palette != NULL);
    memset(palette, 0, sizeof(vga_palette));
}

/**
 * Tint each palette color with a reference color.
 *
 * @param pal Palette to operate on
 * @param ref_index Reference colour index for the mixed color
 * @param start Palette start index
 * @param end Palette end index
 * @param step Strength of the effect (0.0 - 1.0).
 */
void vga_palette_tint_range(vga_palette *pal, vga_index ref_index, vga_index start, vga_index end, float step) {
    vga_color ref = pal->colors[ref_index];
    int u;
    float k;
    for(vga_index i = start; i < end; i++) {
        u = max3(pal->colors[i].r, pal->colors[i].g, pal->colors[i].b);
        k = u / 255.0f * step;
        pal->colors[i].r = clamp(pal->colors[i].r + k * (ref.r - pal->colors[i].r), 0, 255);
        pal->colors[i].g = clamp(pal->colors[i].g + k * (ref.g - pal->colors[i].g), 0, 255);
        pal->colors[i].b = clamp(pal->colors[i].b + k * (ref.b - pal->colors[i].b), 0, 255);
    }
}

/**
 * Mix each palette color together with a reference color.
 *
 * @param pal Palette to operate on
 * @param ref_index Reference colour index for the mixed color
 * @param start Palette start index
 * @param end Palette end index
 * @param step How much is mixed (0.0 - 1.0).
 */
void vga_palette_mix_range(vga_palette *pal, vga_index ref_index, vga_index start, vga_index end, float step) {
    vga_color ref = pal->colors[ref_index];
    float inv = 1 - step;
    for(vga_index i = start; i < end; i++) {
        pal->colors[i].r = clamp((pal->colors[i].r * inv) + (ref.r * step), 0, 255);
        pal->colors[i].g = clamp((pal->colors[i].g * inv) + (ref.g * step), 0, 255);
        pal->colors[i].b = clamp((pal->colors[i].b * inv) + (ref.b * step), 0, 255);
    }
}
