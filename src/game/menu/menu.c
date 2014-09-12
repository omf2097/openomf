#include "video/surface.h"
#include "video/video.h"
#include "game/menu/menu.h"
#include "game/menu/menu_background.h"
#include "audio/sound.h"
#include "utils/log.h"

void menu_create(menu *menu, int x, int y, int w, int h) {
    vector_create(&menu->objs, sizeof(component*));
    menu->x = x;
    menu->y = y;
    menu->w = w;
    menu->h = h;
    menu_background_create(&menu->sur, w, h);
    menu->selected = 0;
    menu->finished = 0;
    
    menu->free_cb = NULL;
    menu->userdata = NULL;
}

void menu_free(menu *menu) {
    vector_free(&menu->objs);
    surface_free(&menu->sur);
}

int menu_get_ypos(menu *menu) {
    int ypos = 8;
    iterator it;
    vector_iter_begin(&menu->objs, &it);
    component **tmp;
    while((tmp = iter_next(&it)) != NULL) {
        ypos += (*tmp)->h;
    }
    return ypos;
}

void menu_select(menu *menu, component *c) {
    component **tmp;
    iterator it;
    int i = 0;
    vector_iter_begin(&menu->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if (*tmp == c) {
            break;
        }
        i++;
    }
    if (tmp == NULL) {
        return;
    }

    tmp = vector_get(&menu->objs, menu->selected);
    (*tmp)->selected=0; // unselect the old component
    if((*tmp)->focus) {
        (*tmp)->focus(*tmp, 0);
    }
    c->selected = 1; //select the new one
    if(c->focus) {
        c->focus(c, 1);
    }

    menu->selected = i;
}

component* menu_selected(menu *menu) {
    component **res = vector_get(&menu->objs, menu->selected);
    return *res;
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
    video_render_sprite(&menu->sur, menu->x, menu->y, BLEND_ALPHA, 0);
    vector_iter_begin(&menu->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        (*tmp)->render(*tmp);
    }
}

int menu_handle_event(menu *menu, SDL_Event *event) {
    // Get selected component
    component **c;
    c = vector_get(&menu->objs, menu->selected);

    if(!(*c)->event(*c, event)) {
        return 0;
    }

    return 1;
}

int menu_handle_action(menu *menu, int action) {
    component **c;
    c = vector_get(&menu->objs, menu->selected);

    if(action == ACT_DOWN || action == ACT_UP) {
        (*c)->selected = 0;
        do {
            if(action == ACT_DOWN) {
                menu->selected++;
            }
            if(action == ACT_UP) {
                menu->selected--;
            }
            // wrap around
            if(menu->selected < 0)
                menu->selected = vector_size(&menu->objs) - 1;
            if(menu->selected >= vector_size(&menu->objs))
                menu->selected = 0;

            // Update selected component
            c = vector_get(&menu->objs, menu->selected);

        } while ((*c)->disabled);
        // Play menu sound
        sound_play(19, 0.5f, 0.0f, 2.0f);
        (*c)->selected = 1;
        return 0;
    }

    if(action == ACT_ESC) {
        menu->finished = 1;
    }

    if(!(*c)->action(*c, action)) {
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

void menu_set_userdata(menu *menu, void *userdata) {
    menu->userdata = userdata;
}

void* menu_get_userdata(menu *menu) {
    return menu->userdata;
}

void menu_set_free_cb(menu *menu, menu_free_cb cb) {
    menu->free_cb = cb;
}
