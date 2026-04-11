#include "video/vga_remap.h"
#include <assert.h>

void vga_remaps_init(vga_remap_tables *remaps) {
    assert(remaps != NULL);
    for(int t = 0; t < VGA_REMAP_COUNT; t++) {
        for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
            remaps->tables[t].data[i] = i;
        }
    }
}

void vga_remap_init(vga_remap_table *remap) {
    assert(remap != NULL);
    for(int i = 0; i < VGA_PALETTE_SIZE; i++) {
        remap->data[i] = i;
    }
}
