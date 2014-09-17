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
    sizer_attach(c, nc);
}

void menu_tick(component *c) {
    menu *m = sizer_get_obj(c);

    // If submenu is set, we need to tick it
    if(m->submenu != NULL && !menu_is_finished(m->submenu)) {
        return component_tick(m->submenu);
    }
}

void menu_render(component *c) {
    sizer *s = component_get_obj(c);
    menu *m = sizer_get_obj(c);

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !menu_is_finished(m->submenu)) {
        return component_render(m->submenu);
    }

    // Otherwise handle this component
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

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !menu_is_finished(m->submenu)) {
        return component_event(m->submenu, event);
    }

    // Otherwise handle this component
    component *c = sizer_get(mc, m->selected);
    if(c != NULL) {
        return component_event(c, event);
    }
    return 1;
}

int menu_action(component *mc, int action) {
    menu *m = sizer_get_obj(mc);
    component *c;

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !menu_is_finished(m->submenu)) {
        return component_action(m->submenu, action);
    }

    // Select last item if ESC is pressed
    if(m->selected == sizer_size(mc)-1 && action == ACT_ESC) {
        // If the last item is already selected, and ESC if punched, change the action to punch
        // This is then passed to the quit (last) component and its callback is called
        // Hacky, but works well in menu sizer.
        m->finished = 1;
        action = ACT_PUNCH;
    } else if(action == ACT_ESC) {
        // Select last item when ESC is pressed and it's not already selected.
        c = sizer_get(mc, sizer_size(mc) - 1);
        menu_select(mc, c);
        return 0;
    }

    // Handle down/up selection movement
    if((c = sizer_get(mc, m->selected)) != NULL) {
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

    // If the key wasn't handled yet and we have a valid component,
    // pass on the event
    if(c != NULL) {
        return component_action(c, action);
    }

    // Tel lthe caller that the event was not handled here.
    return 1;
}

void menu_set_submenu(component *mc, component *submenu) {
    menu *m = sizer_get_obj(mc);
    if(m->submenu) {
        component_free(m->submenu);
    }
    m->submenu = submenu;
    submenu->parent = mc; // Set correct parent
    component_layout(m->submenu, mc->x, mc->y, mc->w, mc->h);
}

component* menu_get_submenu(component *c) {
    menu *m = sizer_get_obj(c);
    return m->submenu;
}

int menu_is_finished(component *c) {
    menu *m = sizer_get_obj(c);
    return m->finished;
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
    int first_selected = 0;
    while((tmp = iter_next(&it)) != NULL) {
        // Select first non-disabled component
        if(!component_is_disabled(*tmp) && !first_selected) {
            component_select(*tmp, 1);
            first_selected = 1;
            m->selected = i;
        }

        // Set component position and size
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
    if(m->submenu) {
        component_free(m->submenu);
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
    sizer_set_tick_cb(c, menu_tick);
    sizer_set_action_cb(c, menu_action);
    sizer_set_layout_cb(c, menu_layout);
    sizer_set_free_cb(c, menu_free);

    return c;
}

