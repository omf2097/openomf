#ifndef VGA_PALETTE_H
#define VGA_PALETTE_H

#include <assert.h>
#include <stdint.h>

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

/**
 * Darken each palette color.
 *
 * @param pal Palette to operate on
 * @param step How much to darken (0 - 255).
 */
void vga_palette_darken(vga_palette *pal, uint8_t step);

/**
 * Mix palette with a target shade of gray.
 * Can be used for under and overmixing by setting blend_factor to
 * extreme values-- output will be clamped (saturate to 0 or 255).
 *
 * @param pal Palette to operate on
 * @param gray what shade of gray to blend towards or away from
 * @param start Palette start index
 * @param end Palette end index
 * @param step how much is mixed (0 = no-op, 256 = full gray)
 */
void vga_palette_light_range(vga_palette *pal, uint8_t gray, vga_index start, vga_index end, int32_t step);

#endif // VGA_PALETTE_H
