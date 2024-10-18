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

#endif // VGA_PALETTE_H
