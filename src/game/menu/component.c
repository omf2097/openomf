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
    c->disabled = (disabled != 0) ? 1 : 0;
}

void component_select(component *c, int selected) {
    c->selected = (selected != 0) ? 1 : 0;
}

void component_click(component *c){
    if(c->on_click) {
        c->on_click(c, c->cb_userdata);
    }
}

void component_toggle(component *c, int option) {
    if(c->on_toggle) {
        c->on_toggle(c, c->cb_userdata, option);
    }
}

void component_slide(component *c, int pos) {
    if(c->on_slide) {
        c->on_slide(c, c->cb_userdata, pos);
    }
}

void component_focus(component *c, int focus) {
    if(c->on_focus) {
        c->on_focus(c, c->cb_userdata, focus);
    }
}

void component_set_onclick(component *c, component_click_cb cb, void *userdata) {
    c->on_click = cb;
    c->cb_userdata = userdata;
}

void component_set_ontoggle(component *c, component_toggle_cb cb, void *userdata) {
    c->on_toggle = cb;
    c->cb_userdata = userdata;
}

void component_set_onslide(component *c, component_slide_cb cb, void *userdata) {
    c->on_slide = cb;
    c->cb_userdata = userdata;
}

void component_set_onfocus(component *c, component_focus_cb cb, void *userdata) {
    c->on_focus = cb;
    c->cb_userdata = userdata;
}

void component_set_id(component *c, int id) {
    c->id = id;
}

int component_get_id(const component *c) {
    return c->id;
}

void component_set_obj(component *c, void *obj) {
    c->obj = obj;
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

void component_create(component *c) {
    c->id = -1;
    c->x = 0;
    c->y = 0;
    c->w = 0;
    c->h = 0;
    c->selected = 0;
    c->disabled = 0;
    c->obj = NULL;
    
    // Function pointers
    c->render = NULL;
    c->action = NULL;
    c->event = NULL;
    c->layout = NULL;
    c->tick = NULL;

    // Event functions
    c->cb_userdata = NULL;
    c->on_click = NULL;
    c->on_toggle = NULL;
    c->on_slide = NULL;
    c->on_focus = NULL;
}

void component_free(component *c) {}
