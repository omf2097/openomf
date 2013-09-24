#ifndef _PALETTE_H
#define _PALETTE_H

typedef struct palette_h {
    unsigned char data[256][3];
    unsigned char remaps[19][256];
} palette;

void fixup_palette(palette *palette);

#endif // _PALETTE_H