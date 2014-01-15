#ifndef _PALETTE_H
#define _PALETTE_H

#include <shadowdive/shadowdive.h>

// globals, yay
extern sd_altpal_file *altpals;

typedef struct palette_h {
    unsigned char data[256][3];
    unsigned char remaps[19][256];
} palette;

int altpals_init();
void altpals_close();
 
void palette_set_player_color(palette *palette, int player, int sourcecolor, int destcolor);
palette* palette_copy(palette *src);

#endif // _PALETTE_H
