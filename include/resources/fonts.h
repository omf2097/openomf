#ifndef _FONTS_H
#define _FONTS_H

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

extern font font_small;
extern font font_large;

int fonts_init();
void fonts_close();

#endif // _FONTS_H