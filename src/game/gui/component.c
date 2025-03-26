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

void component_init(component *c, const gui_theme *theme) {
    component_set_theme(c, theme);
    if(c->init) {
        c->init(c, c->theme);
    }
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

void component_disable(component *c, bool disabled) {
    if(!c->supports_disable)
        return;
    c->is_disabled = (disabled != 0) ? 1 : 0;
}

void component_select(component *c, bool selected) {
    if(!c->supports_select)
        return;
    c->is_selected = (selected != 0) ? 1 : 0;
}

void component_focus(component *c, bool focused) {
    if(!c->supports_focus)
        return;
    c->is_focused = (focused != 0) ? 1 : 0;
    if(c->focus) {
        c->focus(c, c->is_focused == 1);
    }
}

bool component_is_disabled(const component *c) {
    if(!c->supports_disable)
        return 0;
    return c->is_disabled;
}

bool component_is_selected(const component *c) {
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

bool component_is_focused(const component *c) {
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

void component_set_supports(component *c, bool allow_disable, bool allow_select, bool allow_focus) {
    c->supports_select = allow_select;
    c->supports_disable = allow_disable;
    c->supports_focus = allow_focus;
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

void component_set_init_cb(component *c, component_init_cb cb) {
    c->init = cb;
}

void component_set_help_text(component *c, const char *text) {
    if(text == NULL || strlen(text) == 0) {
        return;
    }
    if(c->help != NULL) {
        text_set_from_c(c->help, text);
    } else {
        c->help = text_create_from_c(FONT_SMALL, TEXT_BBOX_MAX, TEXT_BBOX_MAX, text);
    }
}

void component_set_theme(component *c, const gui_theme *theme) {
    c->theme = theme;
}

const gui_theme *component_get_theme(component *c) {
    if(c->theme != NULL) {
        return c->theme;
    }
    assert(false && "Component has no theme");
    return NULL;
}

component *component_create(uint32_t header) {
    component *c = omf_calloc(1, sizeof(component));
    c->header = header;
    c->x_hint = -1;
    c->y_hint = -1;
    c->w_hint = -1;
    c->h_hint = -1;
    c->help = NULL;
    return c;
}

void component_free(component *c) {
    if(c == NULL) {
        return;
    }
    if(c->free != NULL) {
        c->free(c);
    }
    if(c->help != NULL) {
        text_free(&c->help);
    }
    omf_free(c);
}
