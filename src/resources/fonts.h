#ifndef FONTS_H
#define FONTS_H

#include "utils/vector.h"

typedef enum
{
    FONT_BIG,
    FONT_SMALL,
    FONT_NET1,
    FONT_NET2
} font_size;

typedef struct {
    font_size size;
    int w, h;
    vector surfaces;
} font;

extern font font_small;
extern font font_large;
extern font font_net1;
extern font font_net2;

int fonts_init(void);
void fonts_close(void);

#endif // FONTS_H
