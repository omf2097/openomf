#ifndef VGA_REMAP_H
#define VGA_REMAP_H

#include "video/vga_palette.h"
#include <assert.h>

#define VGA_REMAP_COUNT 19

typedef struct vga_remap_table {
    vga_index data[256];
} vga_remap_table;

static_assert(256 == sizeof(vga_remap_table), "vga_palette should pack into 256 bytes");

typedef struct vga_remap_tables {
    vga_remap_table tables[VGA_REMAP_COUNT];
} vga_remap_tables;

static_assert(4864 == sizeof(vga_remap_tables), "vga_remap_tables should pack into 4864 bytes");

void vga_remaps_init(vga_remap_tables *remaps);
void vga_remap_init(vga_remap_table *remap);

#endif // VGA_REMAP_H
