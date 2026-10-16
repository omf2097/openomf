#include "game/gui/xysizer.h"
#include "game/gui/sizer.h"
#include "utils/allocator.h"
#include "utils/log.h"

typedef struct xysizer {
    void *userdata;
} xysizer;

void xysizer_attach(component *c, component *nc, int x, int y, int w, int h) {
    component_set_size_hints(nc, w, h);
    component_set_pos_hints(nc, x, y);
    sizer_attach(c, nc);
}

void xysizer_set_userdata(component *c, void *userdata) {
    xysizer *m = sizer_get_obj(c);
    m->userdata = userdata;
}

void *xysizer_get_userdata(component *c) {
    xysizer *m = sizer_get_obj(c);
    return m->userdata;
}

static void xysizer_render(component *c) {
    // Just render all children
    iterator it;
    component **tmp;
    sizer_begin_iterator(c, &it);
    foreach(it, tmp) {
        component_render(*tmp);
    }
}

static void xysizer_layout(component *c, int x, int y, int w, int h) {
    // Set layout for all components in the sizer
    iterator it;
    component **tmp;
    sizer_begin_iterator(c, &it);
    foreach(it, tmp) {
        // Set component position and size from the component hint
        const vec2i pos_hint = component_get_pos_hint(*tmp);
        const vec2i size_hint = component_get_size_hint(*tmp);
        const int m_x = (pos_hint.x < x) ? x : pos_hint.x;
        const int m_y = (pos_hint.y < y) ? y : pos_hint.y;
        const int m_w = (size_hint.x < 0) ? 0 : size_hint.x;
        const int m_h = (size_hint.y < 0) ? 0 : size_hint.y;
        if(m_w == 0 || m_h == 0) {
            log_debug("Warning: Gui component hidden, because size is 0. Make sure size hints are set!");
        }
        component_layout(*tmp, m_x, m_y, m_w, m_h);
    }
}

static bool xysizer_event(component *c, SDL_Event *event) {
    // Just pass events to all children, stop if it gets handled.
    iterator it;
    component **tmp;
    sizer_begin_iterator(c, &it);
    foreach(it, tmp) {
        if(component_event(*tmp, event)) {
            return true;
        }
    }

    return false;
}

static bool xysizer_action(component *c, int action, int source) {
    // Just pass events to all children
    iterator it;
    component **tmp;
    sizer_begin_iterator(c, &it);
    foreach(it, tmp) {
        if(component_action(*tmp, action, source)) {
            return true;
        }
    }

    return false;
}

static void xysizer_free(component *c) {
    xysizer *m = sizer_get_obj(c);
    omf_free(m);
}

component *xysizer_create(void) {
    component *c = sizer_create();

    xysizer *m = omf_calloc(1, sizeof(xysizer));
    sizer_set_obj(c, m);

    sizer_set_render_cb(c, xysizer_render);
    sizer_set_layout_cb(c, xysizer_layout);
    sizer_set_event_cb(c, xysizer_event);
    sizer_set_action_cb(c, xysizer_action);
    sizer_set_free_cb(c, xysizer_free);

    return c;
}
