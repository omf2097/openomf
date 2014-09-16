#ifndef _MENU_H
#define _MENU_H

#include "video/surface.h"
#include "game/menu/component.h"

typedef struct  {
    surface *bg;
    int selected;
    int obj_h;
    int margin_top;
} menu;

component* menu_create(int obj_h);
void menu_attach(component *menu, component *c);
void menu_select(component *menu, component *c);
component* menu_selected(component *menu);

#endif // _MENU_H
