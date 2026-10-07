#include "game/gui/component.h"
#include "utils/allocator.h"

struct component {
    component_type type; ///< Type tag. The magic enum values also act as a safety header.

    int x;     ///< Horizontal position of the object in pixels. This is in screen coordinates.
    int y;     ///< Vertical position of the object in pixels. This is in screen coordinates.
    int w;     ///< Width of the object in pixels.
    int h;     ///< Height of the object in pixels.
    void *obj; ///< Specialization object pointer. Basically always Sizer or Widget struct.

    int x_hint; ///< X position hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int y_hint; ///< Y position hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int w_hint; ///< W size hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int h_hint; ///< H size hint. Sizers may or may not obey this. -1 = not set. >=0 means set.

    bool supports_select; ///< Whether the component can be selected by component_select() call.
    bool is_selected;     ///< Whether the component is selected

    bool supports_disable; ///< Whether the component can be disabled by component_disable() call.
    bool is_disabled;      ///< Whether the component is disabled

    bool supports_focus; ///< Whether the component can be focused by component_focus() call.
    bool is_focused;     ///< Whether the component is focused

    bool editing; ///< Whether this component is in edit mode and owns input
    bool dirty;   ///< Render inputs changed since last render; a component can use this to skip recompute

    text *help; ///< Help text, if available

    const gui_theme *theme; ///< Theme object. After init, this should be set for all objects.

    component_render_cb render; ///< Render function callback. This tells the component to draw itself.
    component_event_cb event;   ///< Event function callback. Direct SDL2 event handler.
    component_action_cb action; ///< Action function callback. Handles OpenOMF abstract key events.
    component_focus_cb focus;   ///< Focus function callback. Handles OpenOMF focus events.
    component_layout_cb layout; ///< Layout function callback. This is called after the component tree is created. Sets
                                ///< component size and position.
    component_tick_cb tick;     ///< Tick function callback. This is called periodically.
    component_free_cb free;     ///< Free function callback. Any component callbacks should be done here.
    component_find_cb find;     ///< Should only be set by widget and sizer. Used to look up widgets by ID.
    component_init_cb init;     ///< Initialization function callback. This is called right before layout function. This
                                ///< should be used to prerender elements, decide size hints, etc.

    component *parent; ///< Parent component. For widgets, usually a sizer. NULL for root component.
};

bool component_is_sizer(const component *c) {
    return c->type == COMPONENT_SIZER;
}

bool component_is_widget(const component *c) {
    return c->type == COMPONENT_WIDGET;
}

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

bool component_event(component *c, SDL_Event *event) {
    if(c->event) {
        return c->event(c, event);
    }
    return false;
}

bool component_action(component *c, int action, int source) {
    if(c->action) {
        return c->action(c, action, source);
    }
    return false;
}

void component_init(component *c, const gui_theme *theme) {
    component_set_theme(c, theme);
    if(c->init) {
        c->init(c, c->theme);
    }
}

vec2i component_get_pos(const component *c) {
    return vec2i_create(c->x, c->y);
}

vec2i component_get_size(const component *c) {
    return vec2i_create(c->w, c->h);
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
    if(!c->supports_disable) {
        return;
    }
    if(c->is_disabled != disabled) {
        c->is_disabled = disabled;
        c->dirty = true;
    }
}

void component_select(component *c, bool selected) {
    if(!c->supports_select) {
        return;
    }
    if(c->is_selected != selected) {
        c->is_selected = selected;
        c->dirty = true;
    }
}

void component_focus(component *c, bool focused) {
    if(!c->supports_focus) {
        return;
    }
    if(c->is_focused != focused) {
        c->is_focused = focused;
        c->dirty = true;
    }
    if(c->focus) {
        c->focus(c, c->is_focused);
    }
}

bool component_is_disabled(const component *c) {
    if(!c->supports_disable) {
        return 0;
    }
    return c->is_disabled;
}

bool component_is_selected(const component *c) {
    if(!c->supports_select) {
        return 0;
    }
    return c->is_selected;
}

bool component_is_selectable(component *c) {
    return c->supports_select;
}

void component_set_selectable(component *c, bool selectable) {
    if(c->supports_select != selectable) {
        c->supports_select = selectable;
        c->dirty = true;
    }
}

bool component_is_focused(const component *c) {
    if(!c->supports_focus) {
        return 0;
    }
    return c->is_focused;
}

vec2i component_get_pos_hint(const component *c) {
    return vec2i_create(c->x_hint, c->y_hint);
}

vec2i component_get_size_hint(const component *c) {
    return vec2i_create(c->w_hint, c->h_hint);
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
    if(c == NULL || c->find == NULL) {
        return NULL;
    }
    return c->find(c, id);
}

void component_set_obj(component *c, void *obj) {
    c->obj = obj;
}

void *component_get_obj(const component *c) {
    return c->obj;
}

void component_set_supported(component *c, bool allow_disable, bool allow_select, bool allow_focus) {
    c->supports_select = allow_select;
    c->supports_disable = allow_disable;
    c->supports_focus = allow_focus;
}

bool component_is_editing(const component *c) {
    return c->editing;
}

void component_set_editing(component *c, bool editing) {
    if(c->editing != editing) {
        c->editing = editing;
        c->dirty = true;
    }
}

bool component_is_dirty(const component *c) {
    return c->dirty;
}

void component_set_dirty(component *c, bool dirty) {
    c->dirty = dirty;
}

component *component_get_parent(const component *c) {
    return c->parent;
}

void component_set_parent(component *c, component *parent) {
    c->parent = parent;
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
        c->help = text_create_from_c(text);
    }
}

text *component_get_help_text(const component *c) {
    return c->help;
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

component *component_create(component_type type) {
    component *c = omf_calloc(1, sizeof(component));
    c->type = type;
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
    if(c->is_focused != 0) {
        component_focus(c, false);
    }
    if(c->free != NULL) {
        c->free(c);
    }
    if(c->help != NULL) {
        text_free(&c->help);
    }
    omf_free(c);
}
