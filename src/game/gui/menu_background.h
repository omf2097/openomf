#ifndef MENU_BACKGROUND_H
#define MENU_BACKGROUND_H

#include "video/surface.h"

typedef enum menu_background_style
{
    // blue borders, coarse grid
    MenuBackground,
    // green borders, finer grid
    MenuBackgroundMeleeVs,
    // blue borders, no grid
    MenuBackgroundNewsroom,
} menu_background_style;

void menu_transparent_bg_create(surface *s, int w, int h);
void menu_background_create(surface *sur, int w, int h, menu_background_style);
void menu_background_border_create(surface *sur, int w, int h, vga_index color);

#endif // MENU_BACKGROUND_H
