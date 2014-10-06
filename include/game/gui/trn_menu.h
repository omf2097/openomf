#ifndef _TRN_MENU_H
#define _TRN_MENU_H

#include "game/gui/component.h"
#include "resources/animation.h"

typedef void (*trnmenu_tick_cb)(component *c);
typedef void (*trnmenu_free_cb)(component *c);

typedef struct  {
    int selected;

    surface *button_sheet;
    int sheet_x;
    int sheet_y;

    void *userdata;
    trnmenu_free_cb free;
    trnmenu_tick_cb tick;
} trnmenu;

component* trnmenu_create(surface *button_sheet, int sheet_x, int sheet_y);
void trnmenu_attach(component *menu, component *c);

void trnmenu_bind_hand(component *menu, animation *hand);

void trnmenu_set_userdata(component *menu, void *userdata);
void* trnmenu_get_userdata(const component *menu);
void trnmenu_set_free_cb(component *menu, trnmenu_free_cb cb);
void trnmenu_set_tick_cb(component *menu, trnmenu_tick_cb cb);

#endif // _TRN_MENU_H
