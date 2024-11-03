#ifndef MENU_BACKGROUND_H
#define MENU_BACKGROUND_H

#include "video/surface.h"

void menu_transparent_bg_create(surface *s, int w, int h);
void menu_background_create(surface *sur, int w, int h);
void menu_background2_create(surface *sur, int w, int h);
void menu_background_border_create(surface *sur, int w, int h);

#endif // MENU_BACKGROUND_H
