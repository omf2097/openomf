#include "game/gui/menu.h"
#include "audio/audio.h"
#include "game/gui/menu_background.h"
#include "game/gui/sizer.h"
#include "game/gui/text_render.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/vector.h"
#include "video/surface.h"
#include "video/video.h"

void menu_select(component *c, component *sc) {
    menu *m = sizer_get_obj(c);
    component **tmp;
    iterator it;
    int i = 0;
    sizer_begin_iterator(c, &it);
    foreach(it, tmp) {
        if(*tmp == sc) {
            break;
        }
        i++;
    }
    if(tmp == NULL) {
        return;
    }

    // unselect the old component
    component *old = sizer_get(c, m->selected);
    if(old != NULL) {
        component_select(old, 0);
        component_focus(old, 0);
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
    sizer_begin_iterator(c, &it);
    int i = 0;
    foreach(it, tmp) {
        component_render(*tmp);
        if(m->selected == i && (*tmp)->help != NULL) {
            if(m->help_bg1) {
                video_draw_remap(m->help_bg1, m->help_x - 8, m->help_y - 8, 4, 1, 0);
            }
            if(m->help_bg2) {
                video_draw(m->help_bg2, m->help_x - 8, m->help_y - 8);
            }
            text_set_bounding_box((*tmp)->help, m->help_w, m->help_h);
            text_set_color((*tmp)->help, m->help_text_color);
            text_set_horizontal_align((*tmp)->help, m->help_text_halign);
            text_set_vertical_align((*tmp)->help, m->help_text_valign);
            text_set_font((*tmp)->help, m->help_text_font);
            text_draw((*tmp)->help, m->help_x, m->help_y);
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

    if(action == ACT_ESC) {
        bool was_last_selected = m->selected == sizer_size(mc) - 1;
        // Select last item when ESC is pressed
        c = sizer_get(mc, sizer_size(mc) - 1);
        menu_select(mc, c);

        if(m->is_submenu || was_last_selected) {
            // If the last item is already selected, and ESC if punched, change the action to punch
            // This is then passed to the quit (last) component and its callback is called
            // Hacky, but works well in menu sizer.
            m->finished = 1;
            action = ACT_PUNCH;
        } else {
            return 0;
        }
    }

    // Handle down/up selection movement
    c = sizer_get(mc, m->selected);
    if(c != NULL && c->supports_select &&
       (((action == ACT_DOWN || action == ACT_UP) && !m->horizontal) ||
        ((action == ACT_LEFT || action == ACT_RIGHT) && m->horizontal))) {
        component *old_c = c;
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
        if(c != old_c) {
            // Play menu sound
            audio_play_sound(19, 0.5f, 0.0f, 2.0f);
            component_select(c, 1);
        }
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
    menu *subm = sizer_get_obj(submenu);
    subm->is_submenu = true;
    if(m->submenu) {
        component_free(m->submenu);
    }
    m->submenu = submenu;
    m->prev_submenu_state = 0;
    submenu->parent = mc; // Set correct parent
    component_init(m->submenu, component_get_theme(mc));
    component_layout(m->submenu, mc->x, mc->y, mc->w, mc->h);
}

void menu_link_menu(component *mc, gui_frame *linked_menu) {
    menu *m = sizer_get_obj(mc);
    if(m->submenu) {
        component_free(m->submenu);
    }
    int x, y, w, h;
    gui_frame_get_measurements(linked_menu, &x, &y, &w, &h);
    component *root = gui_frame_get_root(linked_menu);
    m->submenu = root;
    m->prev_submenu_state = 0;
    root->parent = mc; // Set correct parent
    component_init(m->submenu, component_get_theme(mc));
    component_layout(m->submenu, x, y, w, h);
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
    menu *m = sizer_get_obj(c);

    // Set layout for all components in the sizer
    iterator it;
    component **tmp;
    sizer_begin_iterator(c, &it);
    int available_space = m->horizontal ? w : h - m->margin_top;
    int non_reserved_space = available_space;
    int non_reserved_items = sizer_size(c);
    foreach(it, tmp) {
        int hint = m->horizontal ? (*tmp)->w_hint : (*tmp)->h_hint;
        if(hint > -1) {
            non_reserved_space -= hint;
            non_reserved_items -= 1;
        }
    }
    // Take padding into account
    non_reserved_space -= m->padding * (sizer_size(c) - 1);

    // This is how much space we have for items that do not declare hint.
    int non_hinted_item_space = non_reserved_items > 0 ? non_reserved_space / non_reserved_items : 0;

    // Select first non-disabled component
    int i = 0;
    sizer_begin_iterator(c, &it);
    foreach(it, tmp) {
        if(!component_is_disabled(*tmp)) {
            component_select(*tmp, 1);
            m->selected = i;
            break;
        }
        i++;
    }

    // Align components. Note that if there are any flexible (non-hinted) items, centering cannot
    // do anything. Therefore only enable when all components declare size hints.
    int offset = 0;
    if(m->centered && non_reserved_items == 0) {
        int reserved_space = available_space - non_reserved_space;
        if(m->horizontal) {
            offset += (w - reserved_space) / 2;
        } else {
            offset += (h - reserved_space) / 2;
        }
    }
    y += m->margin_top;

    // Set layouts!
    sizer_begin_iterator(c, &it);
    foreach(it, tmp) {
        // Set component position and size
        int left = available_space - (offset > 0 ? offset - m->padding : offset);
        if(m->horizontal) {
            int obj_w = (*tmp)->w_hint > -1 ? (*tmp)->w_hint : non_hinted_item_space;
            obj_w = min2(left, obj_w);
            component_layout(*tmp, x + offset, y, obj_w, h);
            offset += obj_w + m->padding;
        } else {
            int obj_h = (*tmp)->h_hint > -1 ? (*tmp)->h_hint : non_hinted_item_space;
            obj_h = min2(left, obj_h);
            component_layout(*tmp, x, y + offset, w, obj_h);
            offset += obj_h + m->padding;
        }
    }
    if(offset > 0) {
        offset -= m->padding;
    }

    int b_axis_max = m->horizontal ? h : w;
    int actual_w = m->horizontal ? offset : b_axis_max;
    int actual_h = m->horizontal ? b_axis_max : offset;

    // Set the background now that we know the width and height
    if(m->bg1 == NULL && m->background) {
        m->bg1 = omf_malloc(sizeof(surface));
        menu_transparent_bg_create(m->bg1, w, actual_h + m->margin_top * 2);
    }
    if(m->bg2 == NULL && m->background) {
        m->bg2 = omf_malloc(sizeof(surface));
        menu_background_create(m->bg2, w, actual_h + m->margin_top * 2, MenuBackground);
    }
    if(m->help_bg1 == NULL && m->background) {
        m->help_bg1 = omf_calloc(1, sizeof(surface));
        menu_transparent_bg_create(m->help_bg1, m->help_w + 16, m->help_w / 8);
    }
    if(m->help_bg2 == NULL && m->background) {
        m->help_bg2 = omf_calloc(1, sizeof(surface));
        menu_background_create(m->help_bg2, m->help_w + 16, m->help_w / 8, MenuBackground);
    }

    component_set_size_hints(c, actual_w, actual_h);
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

void menu_set_background(component *c, bool background) {
    menu *m = sizer_get_obj(c);
    m->background = background;
}

void menu_set_centered(component *c, bool centered) {
    menu *m = sizer_get_obj(c);
    m->centered = centered;
}

void menu_set_help_pos(component *c, int x, int y, int w, int h) {
    menu *m = sizer_get_obj(c);
    m->help_x = x;
    m->help_y = y;
    m->help_w = w;
    m->help_h = h;
}

void menu_set_help_text_settings(component *c, font_size font, text_horizontal_align halign,
                                 vga_index help_text_color) {
    menu *m = sizer_get_obj(c);
    m->help_text_color = help_text_color;
    m->help_text_halign = halign;
    m->help_text_font = font;
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

component *menu_create(void) {
    component *c = sizer_create();

    menu *m = omf_calloc(1, sizeof(menu));
    m->margin_top = 8;
    m->padding = 3;
    m->horizontal = false;
    m->background = true;
    m->centered = false;
    sizer_set_obj(c, m);

    m->help_w = 284;
    m->help_h = 20;
    m->help_x = 16;
    m->help_y = 156;
    m->help_text_color = COLOR_LIGHT_BLUE;
    m->help_text_font = FONT_SMALL;
    m->help_text_halign = TEXT_ALIGN_CENTER;
    m->help_text_valign = TEXT_ALIGN_MIDDLE;

    sizer_set_render_cb(c, menu_render);
    sizer_set_event_cb(c, menu_event);
    sizer_set_tick_cb(c, menu_tick);
    sizer_set_action_cb(c, menu_action);
    sizer_set_layout_cb(c, menu_layout);
    sizer_set_free_cb(c, menu_free);
    sizer_set_find_cb(c, menu_find);

    return c;
}
