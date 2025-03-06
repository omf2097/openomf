#include "game/gui/widget.h"
#include "utils/allocator.h"
#include "utils/log.h"

#define WIDGET_MAGIC 0x8BADF00D

typedef struct widget {
    void *obj; ///< Pointer to internal object, eg. textbutton, etc.
    int id;    ///< Default is -1, which means "not set". User should always set positive values!

    widget_render_cb render;
    widget_event_cb event;
    widget_action_cb action;
    widget_focus_cb focus;
    widget_layout_cb layout;
    widget_tick_cb tick;
    widget_free_cb free;
} widget;

void widget_set_obj(component *c, void *obj) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->obj = obj;
}

void *widget_get_obj(const component *c) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    return local->obj;
}

void widget_set_id(component *c, int id) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->id = id;
}

int widget_get_id(const component *c) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    return local->id;
}

void widget_set_render_cb(component *c, widget_render_cb cb) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->render = cb;
}

void widget_set_event_cb(component *c, widget_event_cb cb) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->event = cb;
}

void widget_set_action_cb(component *c, widget_action_cb cb) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->action = cb;
}

void widget_set_focus_cb(component *c, widget_focus_cb cb) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->focus = cb;
}

void widget_set_layout_cb(component *c, widget_layout_cb cb) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->layout = cb;
}

void widget_set_tick_cb(component *c, widget_tick_cb cb) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->tick = cb;
}

void widget_set_free_cb(component *c, widget_free_cb cb) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    local->free = cb;
}

static void widget_tick(component *c) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    if(local->tick) {
        local->tick(c);
    }
}

static void widget_render(component *c) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    if(local->render) {
        local->render(c);
    }
}

static int widget_event(component *c, SDL_Event *event) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    if(local->event) {
        return local->event(c, event);
    }
    return 1;
}

static int widget_action(component *c, int action) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    if(local->action) {
        return local->action(c, action);
    }
    return 1;
}

void widget_focus(component *c, bool focused) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    if(local->focus) {
        local->focus(c, focused);
    }
}

static void widget_layout(component *c, int x, int y, int w, int h) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    if(local->layout) {
        local->layout(c, x, y, w, h);
    }
}

static void widget_free(component *c) {
    assert(c->header == WIDGET_MAGIC);
    widget *local = component_get_obj(c);
    if(local->free) {
        local->free(c);
    }
    omf_free(local);
}

static component *widget_find(component *c, int id) {
    assert(c->header == WIDGET_MAGIC);
    if(widget_get_id(c) == id) {
        return c;
    }
    return NULL;
}

component *widget_create(void) {
    component *c = component_create();
    c->header = WIDGET_MAGIC;
    c->supports_disable = 1;
    c->supports_select = 1;
    c->supports_focus = 1;

    widget *local = omf_calloc(1, sizeof(widget));
    local->id = -1;
    component_set_obj(c, local);

    component_set_tick_cb(c, widget_tick);
    component_set_render_cb(c, widget_render);
    component_set_event_cb(c, widget_event);
    component_set_action_cb(c, widget_action);
    component_set_focus_cb(c, widget_focus);
    component_set_layout_cb(c, widget_layout);
    component_set_free_cb(c, widget_free);
    component_set_find_cb(c, widget_find);

    return c;
}
