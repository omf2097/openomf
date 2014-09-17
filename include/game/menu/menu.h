#ifndef _MENU_H
#define _MENU_H

#include "video/surface.h"
#include "game/menu/component.h"

typedef void (*menu_free_cb)(component *c);

typedef struct  {
    surface *bg;
    int selected;
    int obj_h;
    int margin_top;
    int finished;

    component *submenu;
    void *userdata;
    menu_free_cb free;
} menu;

component* menu_create(int obj_h);
void menu_attach(component *menu, component *c);
void menu_select(component *menu, component *c);
component* menu_selected(component *menu);
void menu_set_submenu(component *menu, component *submenu);
component* menu_get_submenu(component *menu);
int menu_is_finished(component *menu);

void menu_set_userdata(component *menu, void *userdata);
void* menu_get_userdata(component *menu);
void menu_set_free_cb(component *menu, menu_free_cb cb);

#endif // _MENU_H
