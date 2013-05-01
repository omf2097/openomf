#include "game/menu/menu.h"

#include "utils/log.h"

void menu_create(menu *menu, int x, int y, int w, int h) {
    list_create(&menu->objs);
    menu->x = x;
    menu->y = y;
    menu->w = w;
    menu->h = h;
}

void menu_free(menu *menu) {
    list_free(&menu->objs);
}

int menu_get_ypos(menu *menu) {
    int ypos = 0;
    iterator it;
    list_iter_begin(&menu->objs, &it);
    component **tmp;
    while((tmp = iter_next(&it)) != NULL) {
        ypos += (*tmp)->h;
    }
    return ypos;
}

void menu_attach(menu *menu, component *c, int h) {
    c->layout(c, menu->x, menu->y + menu_get_ypos(menu), menu->w, h);
    list_append(&menu->objs, &c, sizeof(*c));
}

void menu_render(menu *menu) {
    iterator it;
    component **tmp;
    list_iter_begin(&menu->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        (*tmp)->render(*tmp);
    }
}

void menu_handle_event(menu *menu) {
    iterator it;
    component **tmp;
    list_iter_begin(&menu->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        (*tmp)->event(*tmp);
    }
}
