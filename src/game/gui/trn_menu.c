#include "game/gui/trn_menu.h"
#include "game/gui/sizer.h"
#include "video/surface.h"
#include "video/video.h"
#include "utils/vector.h"
#include "utils/log.h"

void trnmenu_attach(component *c, component *nc) {
    sizer_attach(c, nc);
}

static void trnmenu_hand_finished(object *hand_obj) {
    trnmenu_hand *hand = object_get_userdata(hand_obj);
    hand->play = 0;
    player_reset(hand->obj);
    object_dynamic_tick(hand->obj);
}

static int trnmenu_hand_select(component *c) {
    trnmenu *m = sizer_get_obj(c);
    component *sel = sizer_get(c, m->selected);
    if(sel == NULL)
        return 0;
    vec2i pos = vec2i_create(sel->x, sel->y);
    vec2i bsize = vec2i_create(sel->w, sel->h);
    bsize.x /= 2;
    bsize.y /= 2;
    object_set_pos(m->hand.obj, vec2i_add(bsize, pos));
    return 1;
}

void trnmenu_bind_hand(component *c, animation *hand_ani, game_state *gs) {
    trnmenu *m = sizer_get_obj(c);

    // Free old. Shouldn't be needed, but let's be thorough.
    if(m->hand.obj != NULL) {
        object_free(m->hand.obj);
        free(m->hand.obj);
    }

    // Set up new hand object
    m->hand.obj = malloc(sizeof(object));
    object_create(m->hand.obj, gs, vec2i_create(0,0), vec2f_create(0,0));
    object_set_animation(m->hand.obj, hand_ani);
    object_set_userdata(m->hand.obj, &m->hand);
    object_set_finish_cb(m->hand.obj, trnmenu_hand_finished);
    object_dynamic_tick(m->hand.obj);
}

void trnmenu_set_userdata(component *c, void *userdata) {
    trnmenu *m = sizer_get_obj(c);
    m->userdata = userdata;
}

void* trnmenu_get_userdata(const component *c) {
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
        free(m->hand.obj);
    }
    free(m);
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
        // Select first non-disabled component
        if(!component_is_disabled(*tmp) && !first_selected) {
            component_select(*tmp, 1);
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

    // Set hand position
    trnmenu_hand_select(c);
}

/*
static int find_closest(component *c, int act) {

}
*/

static int trnmenu_action(component *c, int action) {
    trnmenu *m = sizer_get_obj(c);
    switch(action) {
        case ACT_LEFT:
            m->selected--;
            if(!trnmenu_hand_select(c)) {
                m->selected++;
            }
            break;
        case ACT_RIGHT:
            m->selected++;
            if(!trnmenu_hand_select(c)) {
                m->selected--;
            }
            break;
        case ACT_UP:
            break;
        case ACT_DOWN:
            break;
        case ACT_ESC:
            break;
        case ACT_PUNCH:
        case ACT_KICK: {
                component *sel = sizer_get(c, m->selected);
                if(sel != NULL) {
                    m->hand.play = 1;
                    return component_action(sel, action);
                }
            }
            break;
    }
    return 0;
}

static void trnmenu_render(component *c) {
    sizer *s = component_get_obj(c);
    trnmenu *m = sizer_get_obj(c);

    // Render button sheet
    video_render_sprite(m->button_sheet, m->sheet_x, m->sheet_y, BLEND_ALPHA, 0);

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
    if(m->hand.play && m->hand.obj) {
        object_dynamic_tick(m->hand.obj);
    }
}

component* trnmenu_create(surface *button_sheet, int sheet_x, int sheet_y) {
    component *c = sizer_create();

    trnmenu* m = malloc(sizeof(trnmenu));
    memset(m, 0, sizeof(trnmenu));
    m->button_sheet = button_sheet;
    m->sheet_x = sheet_x;
    m->sheet_y = sheet_y;
    sizer_set_obj(c, m);

    sizer_set_render_cb(c, trnmenu_render);
    sizer_set_layout_cb(c, trnmenu_layout);
    sizer_set_action_cb(c, trnmenu_action);
    sizer_set_tick_cb(c, trnmenu_tick);
    sizer_set_free_cb(c, trnmenu_free);

    return c;
}