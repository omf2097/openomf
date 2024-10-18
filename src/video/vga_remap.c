#include "video/vga_remap.h"
#include <string.h>

void vga_remaps_init(vga_remap_tables *remaps) {
    memset(remaps, 0, sizeof(vga_remap_tables));
}

void vga_remap_init(vga_remap_table *remap) {
    memset(remap, 0, sizeof(vga_remap_table));
}
