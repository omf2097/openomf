#ifndef VGA_PALETTE_H
#define VGA_PALETTE_H

#include "formats/palette.h"

#define VGA_REMAP_COUNT 19

typedef unsigned char vga_index;

typedef struct vga_color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} __attribute__((packed)) vga_color;

typedef struct vga_palette {
    vga_color colors[256];
} __attribute__((packed)) vga_palette;

typedef struct vga_remap_table {
    vga_index data[256];
} vga_remap_table;

typedef void (*vga_palette_transform)(vga_palette *pal, void *userdata);

typedef struct vga_state {
    vga_palette initial_palette;
    vga_palette current_palette;
    vga_remap_table remaps[VGA_REMAP_COUNT];
    bool dirty_remaps;
    bool dirty_palette;
    unsigned char dirty_range_start;
    unsigned char dirty_range_end;
} vga_state;

void vga_state_init(void);
void vga_state_close(void);

bool vga_is_dirty_palette(vga_index *dirty_range_start, vga_index *dirty_range_end);
bool vga_is_dirty_remaps(void);

void vga_set_initial_state(const palette *src);
void vga_set_palette_index(vga_index index, vga_color color);
void vga_set_palette_indices(vga_index start, vga_index count, vga_color *src_colors);

void vga_apply_palette_transform(vga_palette_transform transform_callback, void *userdata);

#endif // VGA_PALETTE_H
