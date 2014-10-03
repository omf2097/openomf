#include "game/gui/widget.h"


void widget_set_obj(component *c, void *obj) {
    widget *local = component_get_obj(c);
    local->obj = obj;
}

void* widget_get_obj(const component *c) {
    widget *local = component_get_obj(c);
    return local->obj;
}

void widget_set_id(component *c, int id) {
    widget *local = component_get_obj(c);
    local->id = id;
}

int widget_get_id(const component *c) {
    widget *local = component_get_obj(c);
    return local->id;
}

void widget_set_render_cb(component *c, widget_render_cb cb) {
    widget *local = component_get_obj(c);
    local->render = cb;
}

void widget_set_event_cb(component *c, widget_event_cb cb) {
    widget *local = component_get_obj(c);
    local->event = cb;
}

void widget_set_action_cb(component *c, widget_action_cb cb) {
    widget *local = component_get_obj(c);
    local->action = cb;
}

void widget_set_layout_cb(component *c, widget_layout_cb cb) {
    widget *local = component_get_obj(c);
    local->layout = cb;
}

void widget_set_tick_cb(component *c, widget_tick_cb cb) {
    widget *local = component_get_obj(c);
    local->tick = cb;
}

void widget_set_free_cb(component *c, widget_free_cb cb) {
    widget *local = component_get_obj(c);
    local->free = cb;
}

static void widget_tick(component *c) {
    widget *local = component_get_obj(c);
    if(local->tick) {
        local->tick(c);
    }
}

static void widget_render(component *c) {
    widget *local = component_get_obj(c);
    if(local->render) {
        local->render(c);
    }
}

static int widget_event(component *c, SDL_Event *event) {
    widget *local = component_get_obj(c);
    if(local->event) {
        return local->event(c, event);
    }
    return 1;
}

static int widget_action(component *c, int action) {
    widget *local = component_get_obj(c);
    if(local->action) {
        return local->action(c, action);
    }
    return 1;
}

static void widget_layout(component *c, int x, int y, int w, int h) {
    widget *local = component_get_obj(c);
    if(local->layout) {
        local->layout(c, x, y, w, h);
    }
}

static void widget_free(component *c) {
    widget *local = component_get_obj(c);
    if(local->free) {
        local->free(c);
    }
    free(local);
}

component* widget_create() {
    component *c = component_create();
    c->supports_disable = 1;
    c->supports_select = 1;
    c->supports_focus = 1;

    widget *local = malloc(sizeof(widget));
    memset(local, 0, sizeof(widget));
    component_set_obj(c, local);

    component_set_tick_cb(c, widget_tick);
    component_set_render_cb(c, widget_render);
    component_set_event_cb(c, widget_event);
    component_set_action_cb(c, widget_action);
    component_set_layout_cb(c, widget_layout);
    component_set_free_cb(c, widget_free);

    return c;
}

