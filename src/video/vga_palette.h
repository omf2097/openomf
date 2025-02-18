#ifndef VGA_PALETTE_H
#define VGA_PALETTE_H

#include <assert.h>

typedef unsigned char vga_index;

typedef struct vga_color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} vga_color;

static_assert(3 == sizeof(vga_color), "vga_color should pack into 3 bytes");

typedef struct vga_palette {
    vga_color colors[256];
} vga_palette;

static_assert(768 == sizeof(vga_palette), "vga_palette should pack into 768 bytes");

void vga_palette_init(vga_palette *palette);

/**
 * Tint each palette color with a reference color.
 *
 * @param pal Palette to operate on
 * @param ref_index Reference colour index for the mixed color
 * @param start Palette start index
 * @param end Palette end index
 * @param step Strength of the effect (0 - 255).
 */
void vga_palette_tint_range(vga_palette *pal, vga_index ref_index, vga_index start, vga_index end, uint8_t step);

/**
 * Mix each palette color together with a reference color.
 *
 * @param pal Palette to operate on
 * @param ref_index Reference colour index for the mixed color
 * @param start Palette start index
 * @param end Palette end index
 * @param step How much is mixed (0 - 255).
 */
void vga_palette_mix_range(vga_palette *pal, vga_index ref_index, vga_index start, vga_index end, uint8_t step);

#endif // VGA_PALETTE_H
