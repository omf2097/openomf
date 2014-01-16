#ifndef _PROGRESSBAR_H
#define _PROGRESSBAR_H

#include "video/surface.h"
#include "video/color.h"

typedef struct progress_bar_t progress_bar;

#define PROGRESSBAR_LEFT 0
#define PROGRESSBAR_RIGHT 1

struct progress_bar_t {
    surface background;
    surface background_alt;
    surface *block;
    unsigned int w,h,x,y;
    int orientation;
    unsigned int percentage;
    color int_topleft_color;
    color int_bottomright_color;
    color int_bg_color;
};

void progressbar_create_flashing(progress_bar *bar,
                       unsigned int x, unsigned int y,
                       unsigned int w, unsigned int h,
                       color border_topleft_color,
                       color border_bottomright_color,
                       color bg_color,
                       color bg_color_alt,
                       color int_topleft_color,
                       color int_bottomright_color,
                       color int_bg_color,
                       int orientation);
#define progressbar_create(bar, x, y, w, h, border_toplefy, border_bottomright, bg, int_topleft, int_bottomright, int_bg, orientation) progressbar_create_flashing(bar, x, y, w, h, border_toplefy, border_bottomright, bg, bg, int_topleft, int_bottomright, int_bg, orientation)
void progressbar_free(progress_bar *bar);
void progressbar_set(progress_bar *bar, unsigned int percentage);
void progressbar_render_flashing(progress_bar *bar, int flip);
#define progressbar_render(bar) progressbar_render_flashing(bar, 0)

#endif // _PROGRESSBAR_H
