#ifndef _FONTS_H
#define _FONTS_H

#include "utils/vector.h"

typedef enum {
    FONT_BIG,
    FONT_SMALL
} font_size;

typedef struct {
    font_size size;
    int w,h;
    vector surfaces;
} font;

extern font font_small;
extern font font_large;

int fonts_init();
void fonts_close();

#endif // _FONTS_H