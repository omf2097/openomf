#include "game/gui/menu.h"
#include "audio/audio.h"
#include "game/gui/menu_background.h"
#include "game/gui/sizer.h"
#include "game/gui/text_render.h"
#include "utils/allocator.h"
#include "utils/miscmath.h"
#include "utils/vector.h"
#include "video/surface.h"
#include "video/video.h"

void menu_select(component *c, component *sc) {
    sizer *s = component_get_obj(c);
    menu *m = sizer_get_obj(c);
    component **tmp;
    iterator it;
    int i = 0;
    vector_iter_begin(&s->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        if(*tmp == sc) {
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

component *menu_selected(const component *mc) {
    menu *m = sizer_get_obj(mc);
    component *c = sizer_get(mc, m->selected);
    if(c != NULL) {
        return c;
    }
    return NULL;
}

void menu_set_submenu_done_cb(component *c, menu_submenu_done_cb done_cb) {
    menu *m = sizer_get_obj(c);
    m->submenu_done = done_cb;
}

void menu_attach(component *c, component *nc) {
    sizer_attach(c, nc);
}

static void menu_tick(component *c) {
    menu *m = sizer_get_obj(c);

    // If submenu is set, we need to tick it
    if(m->submenu != NULL && !menu_is_finished(m->submenu)) {
        component_tick(m->submenu);
        return;
    }

    // Check if we need to run submenu done -callback
    if(m->submenu != NULL && menu_is_finished(m->submenu) && !m->prev_submenu_state) {
        if(m->submenu_done) {
            m->submenu_done(c, m->submenu);
        }
        m->prev_submenu_state = 1;
    }

    // Run external tick function
    if(m->tick) {
        m->tick(c);
    }
}

static void menu_render(component *c) {
    sizer *s = component_get_obj(c);
    menu *m = sizer_get_obj(c);

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !menu_is_finished(m->submenu)) {
        component_render(m->submenu);
        return;
    }

    // Otherwise handle this component
    iterator it;
    component **tmp;
    if(m->bg1) {
        video_draw_remap(m->bg1, c->x, c->y, 4, 1, 0);
    }
    if(m->bg2) {
        video_draw(m->bg2, c->x, c->y);
    }
    vector_iter_begin(&s->objs, &it);
    int i = 0;
    while((tmp = iter_next(&it)) != NULL) {
        component_render(*tmp);
        if(m->selected == i && (*tmp)->help) {
            if(m->help_bg1) {
                video_draw_remap(m->help_bg1, m->help_x - 8, m->help_y - 8, 4, 1, 0);
            }
            if(m->help_bg2) {
                video_draw(m->help_bg2, m->help_x - 8, m->help_y - 8);
            }
            text_render(&m->help_text_conf, TEXT_DEFAULT, m->help_x, m->help_y, m->help_w, m->help_h, (*tmp)->help);
        }
        i++;
    }
}

static int menu_event(component *mc, SDL_Event *event) {
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

static int menu_action(component *mc, int action) {
    menu *m = sizer_get_obj(mc);
    component *c;

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !menu_is_finished(m->submenu)) {
        return component_action(m->submenu, action);
    }

    // Select last item if ESC is pressed
    if(m->selected == sizer_size(mc) - 1 && action == ACT_ESC) {
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
    c = sizer_get(mc, m->selected);
    if(c != NULL && c->supports_select &&
       (((action == ACT_DOWN || action == ACT_UP) && !m->horizontal) ||
        ((action == ACT_LEFT || action == ACT_RIGHT) && m->horizontal))) {
        component_select(c, 0);
        do {
            if(action == ACT_DOWN && !m->horizontal) {
                m->selected++;
            }
            if(action == ACT_RIGHT && m->horizontal) {
                m->selected++;
            }
            if(action == ACT_UP && !m->horizontal) {
                m->selected--;
            }
            if(action == ACT_LEFT && m->horizontal) {
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
        audio_play_sound(19, 0.5f, 0.0f, 2.0f);
        component_select(c, 1);
        return 0;
    }

    // If the key wasn't handled yet and we have a valid component,
    // pass on the event
    if(c != NULL) {
        return component_action(c, action);
    }

    // Tell the caller that the event was not handled here.
    return 1;
}

void menu_set_submenu(component *mc, component *submenu) {
    menu *m = sizer_get_obj(mc);
    if(m->submenu) {
        component_free(m->submenu);
    }
    m->submenu = submenu;
    m->prev_submenu_state = 0;
    submenu->parent = mc; // Set correct parent
    component_layout(m->submenu, mc->x, mc->y, mc->w_hint, mc->h_hint);
}

void menu_link_menu(component *mc, guiframe *linked_menu) {
    menu *m = sizer_get_obj(mc);
    if(m->submenu) {
        component_free(m->submenu);
    }
    m->submenu = linked_menu->root_node;
    m->prev_submenu_state = 0;
    linked_menu->root_node->parent = mc; // Set correct parent
    component_layout(m->submenu, linked_menu->x, linked_menu->y, linked_menu->w, linked_menu->h);
}

component *menu_get_submenu(const component *c) {
    menu *m = sizer_get_obj(c);
    return m->submenu;
}

int menu_is_finished(const component *c) {
    menu *m = sizer_get_obj(c);
    return m->finished;
}

static void menu_layout(component *c, int x, int y, int w, int h) {
    sizer *s = component_get_obj(c);
    menu *m = sizer_get_obj(c);

    // Set layout for all components in the sizer
    iterator it;
    component **tmp;
    vector_iter_begin(&s->objs, &it);
    int i = 0;
    int first_selected = 0;
    int x_offset = 0;
    int height = 0;
    int centerwidth = 0;
    component *filler = NULL;
    y += m->margin_top;
    while((tmp = iter_next(&it)) != NULL) {
        // Select first non-disabled component
        if(!component_is_disabled(*tmp) && !first_selected) {
            component_select(*tmp, 1);
            first_selected = 1;
            m->selected = i;
        }

        if((*tmp)->filler) {
            filler = (*tmp);
        }

        // Set component position and size
        if(m->horizontal) {
            component_layout(*tmp, x + x_offset, m->margin_top + y, w, -1);
            if(m->centered) {
                centerwidth += (*tmp)->w_hint;
            }
            x_offset += (*tmp)->w_hint + m->padding;
            height = max2(height, (*tmp)->h_hint);
        } else {
            component_layout(*tmp, x, y + height, w, -1);
            height += max2(0, (*tmp)->h_hint) + m->padding;
        }
        i++;
    }

    if(m->horizontal && m->centered) {
        centerwidth += m->padding * (i - 1);
        int x_offset = (w - centerwidth) / 2;
        height = 0;
        vector_iter_begin(&s->objs, &it);
        while((tmp = iter_next(&it)) != NULL) {
            component_layout(*tmp, x + x_offset, m->margin_top + y, w, -1);
            x_offset += (*tmp)->w_hint + m->padding;
            height = max2(height, (*tmp)->h_hint);
        }
    }

    if(!m->horizontal) {
        // get rid of the trailing padding
        height -= m->padding;
    }

    if(filler && filler->h_hint == -1 && h > height) {
        filler->h_hint = h - height;
        menu_layout(c, x, y - m->margin_top, w, h);
        return;
    }

    // Set the background now that we know the width and height
    if(m->bg1 == NULL && m->background) {
        m->bg1 = omf_malloc(sizeof(surface));
        menu_transparent_bg_create(m->bg1, w, height + m->margin_top * 2);
    }
    if(m->bg2 == NULL && m->background) {
        m->bg2 = omf_malloc(sizeof(surface));
        menu_background_create(m->bg2, w, height + m->margin_top * 2);
    }
    if(m->help_bg1 == NULL && m->background) {
        m->help_bg1 = omf_calloc(1, sizeof(surface));
        menu_transparent_bg_create(m->help_bg1, m->help_w + 16, m->help_w / 8);
    }
    if(m->help_bg2 == NULL && m->background) {
        m->help_bg2 = omf_calloc(1, sizeof(surface));
        menu_background_create(m->help_bg2, m->help_w + 16, m->help_w / 8);
    }

    component_set_size_hints(c, w, height);
}

void menu_set_userdata(component *c, void *userdata) {
    menu *m = sizer_get_obj(c);
    m->userdata = userdata;
}

void *menu_get_userdata(const component *c) {
    menu *m = sizer_get_obj(c);
    return m->userdata;
}

void menu_set_free_cb(component *c, menu_free_cb cb) {
    menu *m = sizer_get_obj(c);
    m->free = cb;
}

void menu_set_tick_cb(component *c, menu_tick_cb cb) {
    menu *m = sizer_get_obj(c);
    m->tick = cb;
}

void menu_set_horizontal(component *c, bool horizontal) {
    menu *m = sizer_get_obj(c);
    m->horizontal = horizontal;
}

void menu_set_centered(component *c, bool centered) {
    menu *m = sizer_get_obj(c);
    m->centered = centered;
}

void menu_set_background(component *c, bool background) {
    menu *m = sizer_get_obj(c);
    m->background = background;
}

void menu_set_help_pos(component *c, int x, int y, int w, int h) {
    menu *m = sizer_get_obj(c);
    m->help_x = x;
    m->help_y = y;
    m->help_w = w;
    m->help_h = h;
}

void menu_set_help_text_settings(component *c, text_settings *settings) {
    menu *m = sizer_get_obj(c);
    memcpy(&m->help_text_conf, settings, sizeof(text_settings));
}

static void menu_free(component *c) {
    menu *m = sizer_get_obj(c);
    if(m->bg1) {
        surface_free(m->bg1);
        omf_free(m->bg1);
    }
    if(m->bg2) {
        surface_free(m->bg2);
        omf_free(m->bg2);
    }
    if(m->help_bg1) {
        surface_free(m->help_bg1);
        omf_free(m->help_bg1);
    }
    if(m->help_bg2) {
        surface_free(m->help_bg2);
        omf_free(m->help_bg2);
    }
    if(m->submenu) {
        component_free(m->submenu); // Free submenu component
    }
    if(m->free) {
        m->free(c); // Free menu userdata
    }
    omf_free(m);
}

static component *menu_find(component *c, int id) {
    menu *m = sizer_get_obj(c);
    if(m->submenu) {
        return component_find(m->submenu, id);
    }
    return NULL;
}

void menu_set_margin_top(component *c, int margin) {
    menu *m = sizer_get_obj(c);
    m->margin_top = margin;
}

void menu_set_padding(component *c, int padding) {
    menu *m = sizer_get_obj(c);
    m->padding = padding;
}

component *menu_create(int obj_h) {
    component *c = sizer_create();

    menu *m = omf_calloc(1, sizeof(menu));
    m->margin_top = 8;
    m->padding = 3;
    // m->obj_h = obj_h;
    m->horizontal = false;
    m->background = true;
    m->centered = false;
    sizer_set_obj(c, m);

    m->help_w = 284;
    m->help_h = 20;
    m->help_x = 16;
    m->help_y = 156;

    text_defaults(&m->help_text_conf);
    m->help_text_conf.font = FONT_SMALL;
    m->help_text_conf.halign = TEXT_CENTER;
    m->help_text_conf.valign = TEXT_MIDDLE;
    m->help_text_conf.cforeground = COLOR_LIGHT_BLUE;

    sizer_set_render_cb(c, menu_render);
    sizer_set_event_cb(c, menu_event);
    sizer_set_tick_cb(c, menu_tick);
    sizer_set_action_cb(c, menu_action);
    sizer_set_layout_cb(c, menu_layout);
    sizer_set_free_cb(c, menu_free);
    sizer_set_find_cb(c, menu_find);

    return c;
}
