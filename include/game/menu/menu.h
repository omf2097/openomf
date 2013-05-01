#ifndef _MENU_H
#define _MENU_H

#include "utils/list.h"
#include "game/menu/component.h"

typedef struct menu_t menu;

struct menu_t {
    list objs;
    int x,y,w,h;
};

void menu_create(menu *menu, int x, int y, int w, int h);
void menu_free(menu *menu);
void menu_attach(menu *menu, component *component, int h);
void menu_render(menu *menu); 
void menu_handle_event(menu *menu);

#endif // _MENU_H