#ifndef _MENU_H
#define _MENU_H

#include "video/surface.h"
#include "game/gui/component.h"
#include "game/gui/frame.h"

typedef void (*menu_tick_cb)(component *c);
typedef void (*menu_free_cb)(component *c);
typedef void (*menu_submenu_done_cb)(component *menu, component *submenu);

typedef struct  {
    surface *bg;
    int selected;
    int obj_h;
    int margin_top;
    int finished;

    char prev_submenu_state;
    component *submenu;
    menu_submenu_done_cb submenu_done;

    void *userdata;
    menu_free_cb free;
    menu_tick_cb tick;
} menu;

component* menu_create(int obj_h);
void menu_attach(component *menu, component *c);
void menu_select(component *menu, component *c);
component* menu_selected(const component *menu);
int menu_is_finished(const component *menu);

void menu_set_submenu(component *menu, component *submenu);
void menu_link_menu(component *menu, guiframe *linked_menu);
component* menu_get_submenu(const component *menu);
void menu_set_submenu_done_cb(component *menu, menu_submenu_done_cb done_cb);

void menu_set_userdata(component *menu, void *userdata);
void* menu_get_userdata(const component *menu);
void menu_set_free_cb(component *menu, menu_free_cb cb);
void menu_set_tick_cb(component *menu, menu_tick_cb cb);

#endif // _MENU_H
