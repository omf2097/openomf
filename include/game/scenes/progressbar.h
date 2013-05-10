#ifndef _PROGRESSBAR_H
#define _PROGRESSBAR_H

#include "video/texture.h"
#include "video/color.h"

typedef struct progress_bar_t progress_bar;

#define PROGRESSBAR_LEFT 0
#define PROGRESSBAR_RIGHT 1

struct progress_bar_t {
    texture background;
    texture block;
    unsigned int w,h,x,y;
    int orientation;
    unsigned int percentage;
    color int_topleft_color;
    color int_bottomright_color;
    color int_bg_color;
};

void progressbar_create(progress_bar *bar,     
                       unsigned int x, unsigned int y,
                       unsigned int w, unsigned int h,
                       color border_topleft_color,
                       color border_bottomright_color,
                       color bg_color,
                       color int_topleft_color,
                       color int_bottomright_color,
                       color int_bg_color,
                       int orientation);
void progressbar_free(progress_bar *bar);
void progressbar_set(progress_bar *bar, unsigned int percentage);
void progressbar_render(progress_bar *bar);

#endif // _PROGRESSBAR_H