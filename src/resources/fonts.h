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

void font_create(font *f);
void font_free(font *font);
const surface *font_get_surface(const font *font, char ch);

bool fonts_init(void);
void fonts_close(void);
const font *fonts_get_font(font_size font);

#endif // FONTS_H
