#ifndef _MENU_H
#define _MENU_H

#include "utils/vector.h"
#include "video/surface.h"
#include "game/menu/component.h"
#include <SDL2/SDL.h>

typedef struct menu_t menu;

typedef void (*menu_free_cb)(menu *menu);

struct menu_t {
    vector objs;
    surface sur;
    int x,y,w,h;
    int selected;
    int finished;

    void *userdata;
    menu_free_cb free_cb;
};

void menu_create(menu *menu, int x, int y, int w, int h);
void menu_free(menu *menu);
void menu_attach(menu *menu, component *component, int h);
void menu_select(menu *menu, component *c);
component* menu_selected(menu *menu);
void menu_render(menu *menu);
void menu_tick(menu *menu);
int menu_handle_event(menu *menu, SDL_Event *event);
int menu_handle_action(menu *menu, int action);

void menu_set_userdata(menu *menu, void *userdata);
void* menu_get_userdata(menu *menu);
void menu_set_free_cb(menu *menu, menu_free_cb cb);


#endif // _MENU_H
