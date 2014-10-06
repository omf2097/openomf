#include "game/gui/trn_menu.h"
#include "game/gui/sizer.h"
#include "video/surface.h"
#include "video/video.h"
#include "utils/vector.h"
#include "utils/log.h"

void trnmenu_attach(component *c, component *nc) {
    sizer_attach(c, nc);
}

void trnmenu_bind_hand(component *c, animation *hand) {

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
    sizer_set_free_cb(c, trnmenu_free);

    return c;
}