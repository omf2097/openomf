#include <stdlib.h>

#include "game/menu/component.h"
#include "utils/log.h"

void component_tick(component *c) {
    if(c->tick) {
        c->tick(c);
    }
}

void component_render(component *c) {
    if(c->render) {
        c->render(c);
    }
}

int component_event(component *c, SDL_Event *event) {
    if(c->event) {
        return c->event(c, event);
    }
    return 1;
}

int component_action(component *c, int action) {
    if(c->action) {
        return c->action(c, action);
    }
    return 1;
}

void component_layout(component *c, int x, int y, int w, int h) {
    c->x = x;
    c->y = y;
    c->w = w;
    c->h = h;
    if(c->layout) {
        c->layout(c, x, y, w, h);
    }
}

void component_disable(component *c, int disabled) {
    if(!c->supports_disable)
        return;
    c->is_disabled = (disabled != 0) ? 1 : 0;
}

void component_select(component *c, int selected) {
    if(!c->supports_select)
        return;
    c->is_selected = (selected != 0) ? 1 : 0;
}

void component_focus(component *c, int focused) {
    if(!c->supports_focus)
        return;
    c->is_focused = (focused != 0) ? 1 : 0;
}

int component_is_disabled(const component *c) {
    return c->is_disabled;
}

int component_is_selected(const component *c) {
    return c->is_selected;
}

int component_is_focused(const component *c) {
    return c->is_focused;
}

void component_set_obj(component *c, void *obj) {
    c->obj = obj;
}

void* component_get_obj(const component *c) {
    return c->obj;
}

void component_set_render_cb(component *c, component_render_cb cb) {
    c->render = cb;
}

void component_set_event_cb(component *c, component_event_cb cb) {
    c->event = cb;
}

void component_set_action_cb(component *c, component_action_cb cb) {
    c->action = cb;
}

void component_set_layout_cb(component *c, component_layout_cb cb) {
    c->layout = cb;
}

void component_set_tick_cb(component *c, component_tick_cb cb) {
    c->tick = cb;
}

void component_set_free_cb(component *c, component_free_cb cb) {
    c->free = cb;
}

component* component_create() {
    component *c = malloc(sizeof(component));
    memset(c, 0, sizeof(component));
    return c;
}

void component_free(component *c) {
    if(c == NULL) {
        return;
    }
    if(c->free != NULL) {
        c->free(c);
    }
    free(c);
}
