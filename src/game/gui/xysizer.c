#include "game/gui/xysizer.h"
#include "game/gui/sizer.h"
#include "utils/vector.h"
#include "utils/log.h"

void xysizer_attach(component *c, component *nc, int x, int y, int w, int h) {
    component_set_size_hints(nc, w, h);
    component_set_pos_hints(nc, x, y);
    sizer_attach(c, nc);
}

void xysizer_set_userdata(component *c, void *userdata) {
    xysizer *m = sizer_get_obj(c);
    m->userdata = userdata;
}

void* xysizer_get_userdata(component *c) {
    xysizer *m = sizer_get_obj(c);
    return m->userdata;
}

static void xysizer_render(component *c) {
    sizer *s = component_get_obj(c);

    // Just render all children
    iterator it;
    component **tmp;
    vector_iter_begin(&s->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        component_render(*tmp);
    }
}

static void xysizer_layout(component *c, int x, int y, int w, int h) {
    sizer *s = component_get_obj(c);

    // Set layout for all components in the sizer
    iterator it;
    component **tmp;
    vector_iter_begin(&s->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        // Set component position and size from the component hint
        int m_x = ((*tmp)->x_hint < x) ? x : (*tmp)->x_hint;
        int m_y = ((*tmp)->y_hint < y) ? y : (*tmp)->y_hint;
        int m_w = ((*tmp)->w_hint < 0) ? 0 : (*tmp)->w_hint;
        int m_h = ((*tmp)->h_hint < 0) ? 0 : (*tmp)->h_hint;
        if(m_w == 0 || m_h == 0) {
            DEBUG("Warning: Gui component hidden, because size is 0. Make sure size hints are set!");
        }
        component_layout(*tmp, m_x, m_y, m_w, m_h);
    }
}

static void xysizer_free(component *c) {
    xysizer *m = sizer_get_obj(c);
    free(m);
}

component* xysizer_create(int obj_h) {
    component *c = sizer_create();

    xysizer* m = malloc(sizeof(xysizer));
    memset(m, 0, sizeof(xysizer));
    sizer_set_obj(c, m);

    sizer_set_render_cb(c, xysizer_render);
    sizer_set_layout_cb(c, xysizer_layout);
    sizer_set_free_cb(c, xysizer_free);

    return c;
}

