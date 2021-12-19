#ifndef TRN_MENU_H
#define TRN_MENU_H

#include "game/gui/component.h"
#include "game/protos/object.h"
#include "utils/vec.h"
#include "resources/animation.h"
#include "game/game_state.h"

typedef void (*trnmenu_tick_cb)(component *c);
typedef void (*trnmenu_free_cb)(component *c);
typedef void (*trnmenu_submenu_done_cb)(component *menu, component *submenu);

typedef struct {
    object *obj;
    vec2i pstart;
    vec2i pend;
    float moved;
    int play;
    int move;
} trnmenu_hand;

typedef struct  {
    int selected;

    surface *button_sheet;
    int sheet_x;
    int sheet_y;

    int fade;
    float opacity_step;

    trnmenu_hand hand;

    char prev_submenu_state;
    component *submenu;
    trnmenu_submenu_done_cb submenu_done;
    int finished;

    void *userdata;
    trnmenu_free_cb free;
    trnmenu_tick_cb tick;
} trnmenu;

component* trnmenu_create(surface *button_sheet, int sheet_x, int sheet_y);
void trnmenu_attach(component *menu, component *c);

void trnmenu_bind_hand(component *menu, animation *hand, game_state *gs);

void trnmenu_set_submenu(component *menu, component *submenu);
component* trnmenu_get_submenu(const component *menu);
void trnmenu_set_submenu_done_cb(component *menu, trnmenu_submenu_done_cb done_cb);
int trnmenu_is_finished(const component *menu);
void trnmenu_finish(component *menu);

int trnmenu_is_fading(const component *menu);

void trnmenu_set_userdata(component *menu, void *userdata);
void* trnmenu_get_userdata(const component *menu);
void trnmenu_set_free_cb(component *menu, trnmenu_free_cb cb);
void trnmenu_set_tick_cb(component *menu, trnmenu_tick_cb cb);

#endif // TRN_MENU_H
