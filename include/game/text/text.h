#ifndef _TEXT_H
#define _TEXT_H

#include "utils/vector.h"
#include "video/color.h"

enum {
    FONT_UNDEFINED,
    FONT_BIG,
    FONT_SMALL
};

typedef struct font_t font;

struct font_t {
    int size;
    int w,h;
    vector surfaces;
};

// globals, yay
extern font font_small;
extern font font_large;

int fonts_init();
void fonts_close();
void font_render_char(font *font, char ch, int x, int y, color c);
void font_render_len(font *font, const char *text, int len, int x, int y, color c);
void font_render(font *font, const char *text, int x, int y, color c);
void font_render_wrapped(font *font, const char *text, int x, int y, int w, color c);

#endif // _TEXT_H
