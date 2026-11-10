#include "game/gui/trn_menu.h"
#include "controller/controller.h"
#include "game/gui/component.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/surface.h"
#include "video/video.h"

typedef struct trnmenu_hand {
    object *obj;
    vec2i pstart;
    vec2i pend;
    float moved;
    int play;
    int move;
} trnmenu_hand;

typedef struct {
    int selected;

    surface *button_sheet;
    int sheet_x;
    int sheet_y;
    bool return_hand;

    trnmenu_hand hand;

    component *submenu;
    trnmenu_submenu_init_cb submenu_init;
    trnmenu_submenu_done_cb submenu_done;
    int finished;

    void *userdata;
    trnmenu_free_cb free;
    trnmenu_tick_cb tick;
} trnmenu;

void trnmenu_attach(component *c, component *nc) {
    component_attach(c, nc);
}

static vec2f center(const component *c) {
    const vec2i pos = component_get_pos(c);
    const vec2i size = component_get_size(c);
    return vec2f_create(pos.x + size.x / 2, pos.y + size.y / 2);
}

static vec2f rcenter(const component *c) {
    const vec2i pos = component_get_pos(c);
    const vec2i size = component_get_size(c);
    return vec2f_create(pos.x + size.x, pos.y + size.y / 2);
}

static vec2f lcenter(const component *c) {
    const vec2i pos = component_get_pos(c);
    const vec2i size = component_get_size(c);
    return vec2f_create(pos.x, pos.y + size.y / 2);
}

static void trnmenu_hand_finished(object *hand_obj) {
    trnmenu_hand *hand = object_get_userdata(hand_obj);
    hand->play = 0;
    player_reset(hand->obj);
    object_dynamic_tick(hand->obj);
}

static int trnmenu_hand_deselect(component *c) {
    const trnmenu *m = component_get_obj(c);
    component *sel = component_get_child(c, m->selected);
    if(sel == NULL) {
        return 0;
    }

    component_focus(sel, 0);
    return 1;
}

static int trnmenu_hand_select(component *c) {
    trnmenu *m = component_get_obj(c);
    component *sel = component_get_child(c, m->selected);
    if(sel == NULL) {
        return 0;
    }

    component_focus(sel, 1);
    m->hand.move = 1;
    m->hand.pstart = object_get_pos(m->hand.obj);
    m->hand.pend = vec2f_to_i(center(sel));
    m->hand.moved = 0.0f;
    return 1;
}

void trnmenu_bind_hand(component *c, animation *hand_ani, game_state *gs) {
    trnmenu *m = component_get_obj(c);

    // Free old. Shouldn't be needed, but let's be thorough.
    if(m->hand.obj != NULL) {
        object_free(m->hand.obj);
        omf_free(m->hand.obj);
    }

    // Set up new hand object
    m->hand.obj = omf_calloc(1, sizeof(object));
    object_create(m->hand.obj, gs, vec2i_create(0, 0), vec2f_create(0, 0));
    object_set_animation(m->hand.obj, hand_ani);
    object_set_userdata(m->hand.obj, &m->hand);
    object_set_finish_cb(m->hand.obj, trnmenu_hand_finished);
    object_dynamic_tick(m->hand.obj);
}

void trnmenu_set_userdata(component *c, void *userdata) {
    trnmenu *m = component_get_obj(c);
    m->userdata = userdata;
}

void *trnmenu_get_userdata(const component *c) {
    const trnmenu *m = component_get_obj(c);
    return m->userdata;
}

void trnmenu_set_free_cb(component *c, trnmenu_free_cb cb) {
    trnmenu *m = component_get_obj(c);
    m->free = cb;
}

void trnmenu_set_tick_cb(component *c, trnmenu_tick_cb cb) {
    trnmenu *m = component_get_obj(c);
    m->tick = cb;
}

static void trnmenu_free(component *c) {
    trnmenu *m = component_get_obj(c);
    if(m->free) {
        m->free(c); // Free trnmenu userdata
    }
    if(m->hand.obj != NULL) {
        object_free(m->hand.obj);
        omf_free(m->hand.obj);
    }
    if(m->submenu) {
        component_free(m->submenu);
    }
    omf_free(m);
}

static void trnmenu_layout(component *c, int x, int y, int w, int h) {
    trnmenu *m = component_get_obj(c);

    // Set layout for all components in the sizer
    iterator it;
    component **tmp;
    component_iter_begin(c, &it);
    int i = 0;
    int first_selected = 0;
    foreach(it, tmp) {
        // Select first selectable component
        if(component_is_selectable(*tmp) && !first_selected) {
            component_select(*tmp, 1);
            component_focus(*tmp, 1);
            first_selected = 1;
            m->selected = i;
        }

        // Set component position and size from the component hints (or its required size)
        const vec2i pos_hint = component_get_pos_hint(*tmp);
        const vec2i pref_size = component_get_preferred_size(*tmp);
        const int m_x = (pos_hint.x < x) ? x : pos_hint.x;
        const int m_y = (pos_hint.y < y) ? y : pos_hint.y;
        const int m_w = (pref_size.x < 0) ? 0 : pref_size.x;
        const int m_h = (pref_size.y < 0) ? 0 : pref_size.y;
        if(m_w == 0 || m_h == 0) {
            log_debug("Warning: Gui component hidden, because size is 0. Make sure size hints are set!");
        }
        component_layout(*tmp, m_x, m_y, m_w, m_h);
        i++;
    }

    // Set initial hand position
    component *sel = component_get_child(c, m->selected);
    if(sel != NULL) {
        object_set_pos(m->hand.obj, vec2f_to_i(center(sel)));
    }
}

static int find_next_button(component *c, int act) {
    const trnmenu *m = component_get_obj(c);

    iterator it;
    component **tmp;
    component_iter_begin(c, &it);

    component *cur = component_get_child(c, m->selected);
    float best_dist = 9999.0f;
    int best_idx = -1;
    int idx_now = 0;
    foreach(it, tmp) {
        component *t = *tmp;
        if(!component_is_selectable(t)) {
            idx_now++;
            continue;
        }
        switch(act) {
            case ACT_LEFT:
                if(component_get_pos(t).x < component_get_pos(cur).x) {
                    float tdist = vec2f_dist(rcenter(t), lcenter(cur));
                    if(tdist < best_dist) {
                        best_dist = tdist;
                        best_idx = idx_now;
                    }
                }
                break;
            case ACT_RIGHT:
                if(component_get_pos(t).x > component_get_pos(cur).x) {
                    float tdist = vec2f_dist(lcenter(t), rcenter(cur));
                    if(tdist < best_dist) {
                        best_dist = tdist;
                        best_idx = idx_now;
                    }
                }
                break;
            case ACT_UP:
                if(component_get_pos(t).y < component_get_pos(cur).y) {
                    float tdist = vec2f_dist(center(t), center(cur));
                    if(tdist < best_dist) {
                        best_dist = tdist;
                        best_idx = idx_now;
                    }
                }
                break;
            case ACT_DOWN:
                if(component_get_pos(t).y > component_get_pos(cur).y) {
                    float tdist = vec2f_dist(center(t), center(cur));
                    if(tdist < best_dist) {
                        best_dist = tdist;
                        best_idx = idx_now;
                    }
                }
                break;
        }
        idx_now++;
    }

    return best_idx;
}

static bool trnmenu_action(component *c, int action, int source) {
    trnmenu *m = component_get_obj(c);

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !trnmenu_is_finished(m->submenu)) {
        return component_action(m->submenu, action, source);
    }

    int next;
    switch(action) {
        case ACT_LEFT:
        case ACT_RIGHT:
        case ACT_UP:
        case ACT_DOWN:
            next = find_next_button(c, action);
            if(next != -1 && next != m->selected) {
                trnmenu_hand_deselect(c);
                m->selected = next;
                trnmenu_hand_select(c);
                return true;
            }
            return false;
        case ACT_ESC:
            trnmenu_finish(c);
            return true;
        case ACT_PUNCH:
        case ACT_KICK: {
            component *sel = component_get_child(c, m->selected);
            if(sel != NULL) {
                m->hand.play = 1;
                return component_action(sel, action, source);
            }
            return false;
        }
        default:
            return false;
    }
}

static void trnmenu_render(component *c) {
    const trnmenu *m = component_get_obj(c);

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !trnmenu_is_finished(m->submenu)) {
        component_render(m->submenu);
        return;
    }

    // Render button sheet
    if(m->button_sheet) {
        video_draw(m->button_sheet, m->sheet_x, m->sheet_y);
    }

    // Handle components
    iterator it;
    component **tmp;
    component_iter_begin(c, &it);
    foreach(it, tmp) {
        component_render(*tmp);
    }

    // Render hand if it is set
    if(m->hand.obj != NULL) {
        object_render(m->hand.obj);
    }
}

static void trnmenu_tick(component *c) {
    trnmenu *m = component_get_obj(c);

    // If submenu is set, we need to tick it
    if(m->submenu != NULL && !trnmenu_is_finished(m->submenu)) {
        component_tick(m->submenu);
        return;
    }

    // Check if we need to run submenu done -callback
    if(m->submenu != NULL && trnmenu_is_finished(m->submenu)) {
        component *sel = component_get_child(c, m->selected);
        if(sel != NULL) {
            component_focus(sel, 1);
        }

        if(m->submenu_done) {
            m->submenu_done(c, m->submenu);
        }

        trnmenu *n = component_get_obj(m->submenu);
        if(n->submenu_done) {
            n->submenu_done(c, m->submenu);
        }

        component_free(m->submenu);
        m->submenu = NULL;
    }

    // Tick hand animation
    if(m->hand.play && m->hand.obj) {
        object_dynamic_tick(m->hand.obj);
    }

    // Move hand
    if(m->hand.move && m->hand.obj) {
        // Stop movement if we're done.
        // Otherwise interpolate from target to dest
        if(m->hand.moved >= 1.0) {
            m->hand.move = 0;
            object_set_pos(m->hand.obj, m->hand.pend);
            if(m->return_hand && m->selected != 0) {
                trnmenu_hand_deselect(c);
                m->selected = 0;
                trnmenu_hand_select(c);
            }
        } else {
            vec2i dist = vec2i_sub(m->hand.pend, m->hand.pstart);
            vec2i m_pos = vec2i_create(dist.x * m->hand.moved, dist.y * m->hand.moved);
            vec2i r_pos = vec2i_add(m->hand.pstart, m_pos);
            object_set_pos(m->hand.obj, r_pos);
            m->hand.moved += 0.05f;
        }
    }
}

static bool trnmenu_event(component *mc, SDL_Event *event) {
    const trnmenu *m = component_get_obj(mc);

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !trnmenu_is_finished(m->submenu)) {
        return component_event(m->submenu, event);
    }

    // Otherwise handle this component
    component *c = component_get_child(mc, m->selected);
    if(c != NULL) {
        return component_event(c, event);
    }
    return false;
}

int trnmenu_is_finished(const component *c) {
    const trnmenu *m = component_get_obj(c);
    return m->finished;
}

void trnmenu_set_submenu(component *c, component *submenu) {
    trnmenu *m = component_get_obj(c);
    if(m->submenu) {
        component_free(m->submenu);
    }
    m->submenu = submenu;
    component_set_parent(submenu, c);
    component_init(m->submenu, component_get_theme(c));
    const vec2i pos = component_get_pos(c);
    const vec2i size = component_get_size(c);
    component_layout(m->submenu, pos.x, pos.y, size.x, size.y);

    const trnmenu *n = component_get_obj(submenu);
    if(n->submenu_init) {
        n->submenu_init(c, m->submenu);
    }
}

component *trnmenu_get_submenu(const component *c) {
    const trnmenu *m = component_get_obj(c);
    return m->submenu;
}

void trnmenu_set_submenu_done_cb(component *c, trnmenu_submenu_done_cb done_cb) {
    trnmenu *m = component_get_obj(c);
    m->submenu_done = done_cb;
}

void trnmenu_set_submenu_init_cb(component *c, trnmenu_submenu_init_cb init_cb) {
    trnmenu *m = component_get_obj(c);
    m->submenu_init = init_cb;
}

void trnmenu_finish(component *c) {
    trnmenu *m = component_get_obj(c);
    if(m->submenu == NULL || trnmenu_is_finished(m->submenu)) {
        m->finished = 1;
    }
}

component *trnmenu_create(surface *button_sheet, int sheet_x, int sheet_y, bool return_hand) {
    component *c = component_create();
    component_set_supported(c, false, false, false);

    trnmenu *m = omf_calloc(1, sizeof(trnmenu));
    m->button_sheet = button_sheet;
    m->sheet_x = sheet_x;
    m->sheet_y = sheet_y;
    m->submenu_init = NULL;
    m->submenu_done = NULL;
    m->return_hand = return_hand;
    component_set_obj(c, m);

    component_set_render_cb(c, trnmenu_render);
    component_set_layout_cb(c, trnmenu_layout);
    component_set_action_cb(c, trnmenu_action);
    component_set_event_cb(c, trnmenu_event);
    component_set_tick_cb(c, trnmenu_tick);
    component_set_free_cb(c, trnmenu_free);

    return c;
}
