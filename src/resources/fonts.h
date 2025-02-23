#ifndef FONTS_H
#define FONTS_H

#include "utils/vector.h"
#include "video/surface.h"
#include <stdbool.h>

typedef enum font_size
{
    FONT_BIG,
    FONT_SMALL,
    FONT_NET1,
    FONT_NET2
} font_size;

typedef struct font {
    font_size size;
    int w, h;
    vector surfaces;
} font;

extern font font_small;
extern font font_large;
extern font font_net1;
extern font font_net2;

bool fonts_init(void);
void fonts_close(void);
const font *fonts_get_font(font_size font);
const surface *fonts_get_surface(const font *font, char ch);

#endif // FONTS_H
