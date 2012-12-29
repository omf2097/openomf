#include <stdint.h>
#include "palette.h"

// convert the 6-bit VGA palette into an 8-bit RGB palette
void sd_palette_to_rgb(sd_palette *palette) {
    uint8_t r, g, b;
    for(int i = 0; i < 256; i++) {
        r = (uint8_t)palette->data[i][0];
        palette->data[i][0] = ((r << 2) | (r >> 4));
        g = (uint8_t)palette->data[i][1];
        palette->data[i][1] = ((g << 2) | (g >> 4));
        b = (uint8_t)palette->data[i][2];
        palette->data[i][2] = ((b << 2) | (b >> 4));
    }
}

