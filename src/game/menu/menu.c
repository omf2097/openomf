#include "video/surface.h"
#include "video/video.h"
#include "game/menu/menu.h"
#include "game/menu/menu_background.h"
#include "game/menu/sizer.h"
#include "audio/sound.h"
#include "utils/vector.h"
#include "utils/log.h"

void menu_select(component *c, component *sc) {
    sizer *s = component_get_obj(c);
    menu *m = sizer_get_obj(c);
    component **tmp;
    iterator it;
    int i = 0;
    vector_iter_begin(&s->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if (*tmp == sc) {
            break;
        }
        i++;
    }
    if(tmp == NULL) {
        return;
    }

    // unselect the old component
    tmp = vector_get(&s->objs, m->selected);
    if(tmp != NULL) {
        component_select(*tmp, 0);
        component_focus(*tmp, 0);
    }

    // Select the new component
    component_select(sc, 1);
    component_focus(sc, 1);
    m->selected = i;
}

component* menu_selected(component *mc) {
    menu *m = sizer_get_obj(mc);
    component *c = sizer_get(mc, m->selected);
    if(c != NULL) {
        return c;
    }
    return NULL;
}

void menu_attach(component *c, component *nc) {
    sizer *s = component_get_obj(c);
    if(sizer_size(c) == 0) {
        component_select(nc, 1);
    }
    vector_append(&s->objs, &nc);
}

void menu_render(component *c) {
    sizer *s = component_get_obj(c);
    menu *m = sizer_get_obj(c);
    iterator it;
    component **tmp;
    video_render_sprite(m->bg, c->x, c->y, BLEND_ALPHA, 0);
    vector_iter_begin(&s->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        component_render(*tmp);
    }
}

int menu_event(component *mc, SDL_Event *event) {
    menu *m = sizer_get_obj(mc);
    component *c = sizer_get(mc, m->selected);
    if(c != NULL) {
        return component_event(c, event);
    }
    return 1;
}

int menu_action(component *mc, int action) {
    menu *m = sizer_get_obj(mc);

    component *c = sizer_get(mc, m->selected);
    if(c != NULL) {
        if(action == ACT_DOWN || action == ACT_UP) {
            component_select(c, 0);
            do {
                if(action == ACT_DOWN) {
                    m->selected++;
                }
                if(action == ACT_UP) {
                    m->selected--;
                }
                // wrap around
                if(m->selected < 0)
                    m->selected = sizer_size(mc) - 1;
                if(m->selected >= sizer_size(mc))
                    m->selected = 0;

                // Update selected component
                c = sizer_get(mc, m->selected);

            } while(component_is_disabled(c));
            // Play menu sound
            sound_play(19, 0.5f, 0.0f, 2.0f);
            component_select(c, 1);
            return 0;
        }
    }

    if(m->selected == sizer_size(mc) && action == ACT_ESC) {
        // Finish up this menu
    } else if(action == ACT_ESC) {
        c = sizer_get(mc, sizer_size(mc) - 1);
        menu_select(mc, c);
    }

    if(c != NULL) {
        return component_action(c, action);
    }

    return 1;
}

void menu_layout(component *c, int x, int y, int w, int h) {
    sizer *s = component_get_obj(c);
    menu *m = sizer_get_obj(c);

    // Set the background now that we know the width and height
    if(m->bg == NULL) {
        m->bg = malloc(sizeof(surface));
        menu_background_create(m->bg, w, h);
    }

    // Set layout for all components in the sizer
    iterator it;
    component **tmp;
    vector_iter_begin(&s->objs, &it);
    int i = 0;
    while((tmp = iter_next(&it)) != NULL) {
        component_layout(*tmp, x, m->margin_top + y + i * m->obj_h, w, m->obj_h);
        i++;
    }
}

void menu_free(component *c) {
    menu *m = sizer_get_obj(c);
    if(m->bg) {
        surface_free(m->bg);
        free(m->bg);
    }
}

component* menu_create(int obj_h) {
    component *c = sizer_create();

    menu* m = malloc(sizeof(menu));
    memset(m, 0, sizeof(menu));
    m->margin_top = 8;
    m->obj_h = obj_h;
    sizer_set_obj(c, m);

    sizer_set_render_cb(c, menu_render);
    sizer_set_event_cb(c, menu_event);
    sizer_set_action_cb(c, menu_action);
    sizer_set_layout_cb(c, menu_layout);
    sizer_set_free_cb(c, menu_free);

    return c;
}

