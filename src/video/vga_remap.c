#include "video/vga_remap.h"
#include <assert.h>
#include <string.h>

void vga_remaps_init(vga_remap_tables *remaps) {
    assert(remaps != NULL);
    memset(remaps, 0, sizeof(vga_remap_tables));
    for(int i = 0; i < VGA_REMAP_COUNT; i++) {
        remaps->tables[i].type = VGA_PALETTE_VGA256;
    }
}

void vga_remap_init(vga_remap_table *remap) {
    assert(remap != NULL);
    memset(remap, 0, sizeof(vga_remap_table));
    remap->type = VGA_PALETTE_VGA256;
}
