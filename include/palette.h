#ifndef _PALETTE_H
#define _PALETTE_H

typedef struct palette_t {
    char data[256][3];
    char remaps[19][256];
} sd_palette;

#endif // _PALETTE_H
