#ifndef _MENU_H
#define _MENU_H

#include "utils/list.h"
#include "game/menu/component.h"

typedef struct menu_t menu;

struct menu_t {
    list objs; // List of all components
};

void menu_create(menu *menu);
void menu_free(menu *menu);

/*
* This attachs the given component to the menu.
* Any rendering request to the menu will also be directed to all attached objects.
* Any events will be directed to the relevant components only.
*/
void menu_attach(menu *menu, component *component, int x, int y, int w, int h);

void menu_render(menu *menu); // Renders menu on texture (or something else ? FBO ?)
void menu_handle_event(menu *menu); // Handle SDL event, pass to relevant components

#endif // _MENU_H