#include "video/vga_remap.h"
#include <assert.h>
#include <string.h>

void vga_remaps_init(vga_remap_tables *remaps) {
    assert(remaps != NULL);
    memset(remaps, 0, sizeof(vga_remap_tables));
}

void vga_remap_init(vga_remap_table *remap) {
    assert(remap != NULL);
    memset(remap, 0, sizeof(vga_remap_table));
}
