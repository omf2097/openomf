#ifndef _PALETTE_H
#define _PALETTE_H

typedef struct palette_t {
    char data[256][3];
    char remaps[19][256];
} sd_palette;

void sd_palette_to_rgb(sd_palette *palette);

#endif // _PALETTE_H
