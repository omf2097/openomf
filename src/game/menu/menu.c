#include "game/menu/menu.h"

#include "utils/log.h"

void menu_create(menu *menu, int x, int y, int w, int h) {
    vector_create(&menu->objs, sizeof(component*));
    menu->x = x;
    menu->y = y;
    menu->w = w;
    menu->h = h;
    menu->selected = 0;
}

void menu_free(menu *menu) {
    vector_free(&menu->objs);
}

int menu_get_ypos(menu *menu) {
    int ypos = 0;
    iterator it;
    vector_iter_begin(&menu->objs, &it);
    component **tmp;
    while((tmp = iter_next(&it)) != NULL) {
        ypos += (*tmp)->h;
    }
    return ypos;
}

void menu_attach(menu *menu, component *c, int h) {
    c->layout(c, menu->x, menu->y + menu_get_ypos(menu), menu->w, h);
    if(vector_size(&menu->objs) == 0) {
        c->selected = 1;
    }
    vector_append(&menu->objs, &c);
}

void menu_render(menu *menu) {
    iterator it;
    component **tmp;
    vector_iter_begin(&menu->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        (*tmp)->render(*tmp);
    }
}

int menu_handle_event(menu *menu, SDL_Event *event) {
    // Get selected component
    component **c;
    c = vector_get(&menu->objs, menu->selected);

    // Handle selection
    switch(event->type) {
        case SDL_KEYDOWN:
            if(event->key.keysym.sym == SDLK_DOWN || event->key.keysym.sym == SDLK_UP) {
                (*c)->selected = 0;
                
                if(event->key.keysym.sym == SDLK_DOWN) {
                    menu->selected++;
                }
                if(event->key.keysym.sym == SDLK_UP) {
                    menu->selected--;
                }
                if(menu->selected < 0) menu->selected = 0;
                if(menu->selected >= vector_size(&menu->objs)) menu->selected = vector_size(&menu->objs) - 1;
                
                // Update selected component
                c = vector_get(&menu->objs, menu->selected);
                (*c)->selected = 1;
                return 0;
            }
    }

    if(!(*c)->event(*c)) {
        return 0;
    }
    
    return 1;
}

void menu_tick(menu *menu) {
    iterator it;
    component **tmp;
    vector_iter_begin(&menu->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        (*tmp)->tick(*tmp);
    }
}
