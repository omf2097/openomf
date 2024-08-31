#include "game/gui/trn_menu.h"
#include "game/gui/sizer.h"
#include "game/gui/text_render.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "utils/miscmath.h"
#include "utils/vector.h"
#include "video/surface.h"
#include "video/video.h"

#define OPACITY_STEP 0.03f

void trnmenu_attach(component *c, component *nc) {
    sizer_attach(c, nc);
}

static void trnmenu_hand_finished(object *hand_obj) {
    trnmenu_hand *hand = object_get_userdata(hand_obj);
    hand->play = 0;
    player_reset(hand->obj);
    object_dynamic_tick(hand->obj);
}

static int trnmenu_hand_deselect(component *c) {
    trnmenu *m = sizer_get_obj(c);
    component *sel = sizer_get(c, m->selected);
    if(sel == NULL)
        return 0;

    DEBUG("defocusing button");
    component_focus(sel, 0);
    return 1;
}

static int trnmenu_hand_select(component *c) {
    trnmenu *m = sizer_get_obj(c);
    component *sel = sizer_get(c, m->selected);
    if(sel == NULL)
        return 0;

    DEBUG("defocusing button");
    component_focus(sel, 1);
    m->hand.move = 1;
    m->hand.pstart = object_get_pos(m->hand.obj);
    m->hand.pend = vec2i_create(sel->x + sel->w / 2, sel->y + sel->h / 2);
    m->hand.moved = 0.0f;
    return 1;
}

void trnmenu_bind_hand(component *c, animation *hand_ani, game_state *gs) {
    trnmenu *m = sizer_get_obj(c);

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
    trnmenu *m = sizer_get_obj(c);
    m->userdata = userdata;
}

void *trnmenu_get_userdata(const component *c) {
    trnmenu *m = sizer_get_obj(c);
    return m->userdata;
}

void trnmenu_set_free_cb(component *c, trnmenu_free_cb cb) {
    trnmenu *m = sizer_get_obj(c);
    m->free = cb;
}

void trnmenu_set_tick_cb(component *c, trnmenu_tick_cb cb) {
    trnmenu *m = sizer_get_obj(c);
    m->tick = cb;
}

static void trnmenu_free(component *c) {
    trnmenu *m = sizer_get_obj(c);
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
    sizer *s = component_get_obj(c);
    trnmenu *m = sizer_get_obj(c);

    // Set layout for all components in the sizer
    iterator it;
    component **tmp;
    vector_iter_begin(&s->objs, &it);
    int i = 0;
    int first_selected = 0;
    while((tmp = iter_next(&it)) != NULL) {
        // Select first selectable component
        if(component_is_selectable(*tmp) && !first_selected) {
            component_select(*tmp, 1);
            component_focus(*tmp, 1);
            first_selected = 1;
            m->selected = i;
        }

        // Set component position and size from the component hint
        int m_x = ((*tmp)->x_hint < x) ? x : (*tmp)->x_hint;
        int m_y = ((*tmp)->y_hint < y) ? y : (*tmp)->y_hint;
        int m_w = ((*tmp)->w_hint < 0) ? 0 : (*tmp)->w_hint;
        int m_h = ((*tmp)->h_hint < 0) ? 0 : (*tmp)->h_hint;
        if(m_w == 0 || m_h == 0) {
            DEBUG("Warning: Gui component hidden, because size is 0. Make sure size hints are set!");
        }
        component_layout(*tmp, m_x, m_y, m_w, m_h);
        i++;
    }

    // Set initial hand position
    component *sel = sizer_get(c, m->selected);
    object_set_pos(m->hand.obj, vec2i_create(sel->x + sel->w / 2, sel->y + sel->h / 2));
}

static vec2f center(component *c) {
    return vec2f_create(c->x + c->w / 2, c->y + c->h / 2);
}

static vec2f rcenter(component *c) {
    return vec2f_create(c->x + c->w, c->y + c->h / 2);
}

static vec2f lcenter(component *c) {
    return vec2f_create(c->x, c->y + c->h / 2);
}

static int find_next_button(component *c, int act) {
    sizer *s = component_get_obj(c);
    trnmenu *m = sizer_get_obj(c);

    iterator it;
    component **tmp;
    vector_iter_begin(&s->objs, &it);

    component *cur = sizer_get(c, m->selected);
    float best_dist = 9999.0f;
    int best_idx = -1;
    int idx_now = 0;
    while((tmp = iter_next(&it)) != NULL) {
        component *t = *tmp;
        if(!component_is_selectable(t)) {
            idx_now++;
            continue;
        }
        switch(act) {
            case ACT_LEFT:
                if(t->x < cur->x) {
                    float tdist = vec2f_dist(rcenter(t), lcenter(cur));
                    if(tdist < best_dist) {
                        best_dist = tdist;
                        best_idx = idx_now;
                    }
                }
                break;
            case ACT_RIGHT:
                if(t->x > cur->x) {
                    float tdist = vec2f_dist(lcenter(t), rcenter(cur));
                    if(tdist < best_dist) {
                        best_dist = tdist;
                        best_idx = idx_now;
                    }
                }
                break;
            case ACT_UP:
                if(t->y < cur->y) {
                    float tdist = vec2f_dist(center(t), center(cur));
                    if(tdist < best_dist) {
                        best_dist = tdist;
                        best_idx = idx_now;
                    }
                }
                break;
            case ACT_DOWN:
                if(t->y > cur->y) {
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

static int trnmenu_action(component *c, int action) {
    trnmenu *m = sizer_get_obj(c);

    // If fading, wait until it's done.
    if(m->fade) {
        return 1;
    }

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !trnmenu_is_finished(m->submenu)) {
        return component_action(m->submenu, action);
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
            }
            break;
        case ACT_ESC:
            trnmenu_finish(c);
            break;
        case ACT_PUNCH:
        case ACT_KICK: {
            component *sel = sizer_get(c, m->selected);
            if(sel != NULL) {
                m->hand.play = 1;
                return component_action(sel, action);
            }
        } break;
    }
    return 0;
}

static void trnmenu_render(component *c) {
    sizer *s = component_get_obj(c);
    trnmenu *m = sizer_get_obj(c);

    // If submenu is set, we need to use it
    if(!m->fade && m->submenu != NULL && !trnmenu_is_finished(m->submenu)) {
        return component_render(m->submenu);
    }

    // Render button sheet
    if(m->button_sheet) {
        video_render_sprite_flip_scale_opacity_tint(m->button_sheet, m->sheet_x, m->sheet_y, BLEND_ALPHA, 0, FLIP_NONE,
                                                    1.0f, 1.0f, clamp(s->opacity * 255, 0, 255),
                                                    color_create(0xFF, 0xFF, 0xFF, 0xFF));
    }

    // Handle components
    iterator it;
    component **tmp;
    vector_iter_begin(&s->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        component_render(*tmp);
    }

    // Render hand if it is set
    if(m->hand.obj != NULL) {
        object_render(m->hand.obj);
    }
}

static void trnmenu_tick(component *c) {
    trnmenu *m = sizer_get_obj(c);
    sizer *s = component_get_obj(c);

    // If fade is not ongoing, try to handle submenu. If fade IS ongoing, handle it.
    if(!m->fade) {
        // If submenu is set, we need to tick it
        if(m->submenu != NULL && !trnmenu_is_finished(m->submenu)) {
            return component_tick(m->submenu);
        }
    } else {
        // Tick opacity
        s->opacity += m->opacity_step;

        // Check if fade is done, and set tick to 0 if so.
        if(m->opacity_step > 0 && s->opacity >= 1.0f) {
            s->opacity = 1.0f;
            m->fade = 0;
        } else if(m->opacity_step < 0 && s->opacity <= 0.0f) {
            s->opacity = 0.0f;
            m->fade = 0;
            if(m->submenu == NULL || trnmenu_is_finished(m->submenu)) {
                m->finished = 1;
            }
        }
    }

    // Check if we need to run submenu done -callback
    // Also reset fade
    if(m->submenu != NULL && trnmenu_is_finished(m->submenu) && !trnmenu_is_fading(m->submenu)) {
        if(!m->prev_submenu_state) {
            m->fade = 1;
            s->opacity = 0;
            m->opacity_step = OPACITY_STEP;
            component *sel = sizer_get(c, m->selected);
            if(sel != NULL)
                component_focus(sel, 1);

            if(m->submenu_done) {
                m->submenu_done(c, m->submenu);
            }

            trnmenu *n = sizer_get_obj(m->submenu);
            if(n->submenu_done) {
                n->submenu_done(c, m->submenu);
            }

            m->prev_submenu_state = 1;
        }
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

int trnmenu_is_fading(const component *c) {
    trnmenu *m = sizer_get_obj(c);
    return m->fade;
}

static int trnmenu_event(component *mc, SDL_Event *event) {
    trnmenu *m = sizer_get_obj(mc);

    // If fading, wait until it's done.
    if(m->fade) {
        return 1;
    }

    // If submenu is set, we need to use it
    if(m->submenu != NULL && !trnmenu_is_finished(m->submenu)) {
        return component_event(m->submenu, event);
    }

    // Otherwise handle this component
    component *c = sizer_get(mc, m->selected);
    if(c != NULL) {
        return component_event(c, event);
    }
    return 1;
}

int trnmenu_is_finished(const component *c) {
    trnmenu *m = sizer_get_obj(c);
    return m->finished;
}

void trnmenu_set_submenu(component *c, component *submenu) {
    trnmenu *m = sizer_get_obj(c);
    if(m->submenu) {
        component_free(m->submenu);
    }
    m->submenu = submenu;
    m->prev_submenu_state = 0;
    submenu->parent = c; // Set correct parent
    component_layout(m->submenu, c->x, c->y, c->w, c->h);

    trnmenu *n = sizer_get_obj(submenu);
    if(n->submenu_init) {
        n->submenu_init(c, m->submenu);
    }

    m->opacity_step = -OPACITY_STEP;
    m->fade = 1;
}

component *trnmenu_get_submenu(const component *c) {
    trnmenu *m = sizer_get_obj(c);
    return m->submenu;
}

void trnmenu_set_submenu_done_cb(component *c, trnmenu_submenu_done_cb done_cb) {
    trnmenu *m = sizer_get_obj(c);
    m->submenu_done = done_cb;
}

void trnmenu_set_submenu_init_cb(component *c, trnmenu_submenu_init_cb init_cb) {
    trnmenu *m = sizer_get_obj(c);
    m->submenu_init = init_cb;
}

void trnmenu_finish(component *c) {
    trnmenu *m = sizer_get_obj(c);
    m->fade = 1;
    m->opacity_step = -OPACITY_STEP;
}

component *trnmenu_create(surface *button_sheet, int sheet_x, int sheet_y, bool return_hand) {
    component *c = sizer_create();

    trnmenu *m = omf_calloc(1, sizeof(trnmenu));
    m->button_sheet = button_sheet;
    m->sheet_x = sheet_x;
    m->sheet_y = sheet_y;
    m->fade = 1;
    m->submenu_init = NULL;
    m->submenu_done = NULL;
    m->opacity_step = OPACITY_STEP;
    m->return_hand = return_hand;
    sizer_set_obj(c, m);

    sizer_set_render_cb(c, trnmenu_render);
    sizer_set_layout_cb(c, trnmenu_layout);
    sizer_set_action_cb(c, trnmenu_action);
    sizer_set_event_cb(c, trnmenu_event);
    sizer_set_tick_cb(c, trnmenu_tick);
    sizer_set_free_cb(c, trnmenu_free);

    return c;
}
