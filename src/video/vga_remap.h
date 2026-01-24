#ifndef VGA_REMAP_H
#define VGA_REMAP_H

#include "video/vga_palette.h"
#include <assert.h>

#define VGA_REMAP_COUNT 19

typedef struct vga_remap_table {
    vga_index data[1024];
    vga_palette_type type;
} vga_remap_table;

static_assert(1028 == sizeof(vga_remap_table), "vga_remap_table should pack into 1028 bytes");

typedef struct vga_remap_tables {
    vga_remap_table tables[VGA_REMAP_COUNT];
} vga_remap_tables;

static_assert(19532 == sizeof(vga_remap_tables), "vga_remap_tables should pack into 19532 bytes");

void vga_remaps_init(vga_remap_tables *remaps);
void vga_remap_init(vga_remap_table *remap);

#endif // VGA_REMAP_H
