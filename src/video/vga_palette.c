#include "video/vga_palette.h"
#include <string.h>

void vga_palette_init(vga_palette *palette) {
    memset(palette, 0, sizeof(vga_palette));
}
