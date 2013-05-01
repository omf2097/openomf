#ifndef _MENU_H
#define _MENU_H

#include "utils/vector.h"
#include "game/menu/component.h"
#include <SDL2/SDL.h>

typedef struct menu_t menu;

struct menu_t {
    vector objs;
    int x,y,w,h;
    int selected;
};

void menu_create(menu *menu, int x, int y, int w, int h);
void menu_free(menu *menu);
void menu_attach(menu *menu, component *component, int h);
void menu_render(menu *menu);
void menu_tick(menu *menu);
int menu_handle_event(menu *menu, SDL_Event *event);

#endif // _MENU_H