#ifndef VGA_REMAP_H
#define VGA_REMAP_H

#include "video/vga_palette.h"
#include <assert.h>

#define VGA_REMAP_COUNT 19

typedef struct vga_remap_table {
    vga_index data[VGA_PALETTE_SIZE];
} vga_remap_table;

static_assert((sizeof(vga_index) * VGA_PALETTE_SIZE) == sizeof(vga_remap_table), "vga_remap_table should pack properly");

typedef struct vga_remap_tables {
    vga_remap_table tables[VGA_REMAP_COUNT];
} vga_remap_tables;

static_assert((sizeof(vga_index) * VGA_REMAP_COUNT * VGA_PALETTE_SIZE) == sizeof(vga_remap_tables),
              "vga_remap_tables should pack properly");

void vga_remaps_init(vga_remap_tables *remaps);
void vga_remap_init(vga_remap_table *remap);

#endif // VGA_REMAP_H
