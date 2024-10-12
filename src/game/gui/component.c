#include <stdlib.h>

#include "game/gui/component.h"
#include "utils/allocator.h"
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
    if(c->focus) {
        DEBUG("running component focus cb");
        c->focus(c, c->is_focused == 1);
    }
}

int component_is_disabled(const component *c) {
    if(!c->supports_disable)
        return 0;
    return c->is_disabled;
}

int component_is_selected(const component *c) {
    if(!c->supports_select)
        return 0;
    return c->is_selected;
}

bool component_is_selectable(component *c) {
    if(!c->supports_select) {
        return false;
    }
    return true;
}

int component_is_focused(const component *c) {
    if(!c->supports_focus)
        return 0;
    return c->is_focused;
}

void component_set_size_hints(component *c, int w, int h) {
    c->w_hint = w;
    c->h_hint = h;
}

void component_set_pos_hints(component *c, int x, int y) {
    c->x_hint = x;
    c->y_hint = y;
}

component *component_find(component *c, int id) {
    return c->find(c, id);
}

void component_set_obj(component *c, void *obj) {
    c->obj = obj;
}

void *component_get_obj(const component *c) {
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

void component_set_focus_cb(component *c, component_focus_cb cb) {
    c->focus = cb;
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

void component_set_find_cb(component *c, component_find_cb cb) {
    c->find = cb;
}

void component_set_help_text(component *c, const char *help) {
    c->help = help;
}

component *component_create(void) {
    component *c = omf_calloc(1, sizeof(component));
    c->x_hint = -1;
    c->y_hint = -1;
    c->w_hint = -1;
    c->h_hint = -1;
    c->help = NULL;
    c->filler = false;
    return c;
}

void component_free(component *c) {
    if(c == NULL) {
        return;
    }
    if(c->free != NULL) {
        c->free(c);
    }
    omf_free(c);
}
