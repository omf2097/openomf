#include "video/vga_palette.h"
#include <assert.h>
#include <string.h>

void vga_palette_init(vga_palette *palette) {
    assert(palette != NULL);
    memset(palette, 0, sizeof(vga_palette));
}
