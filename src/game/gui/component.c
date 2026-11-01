#include "game/gui/component.h"
#include "utils/allocator.h"
#include "utils/vector.h"

struct component {
    int x;     ///< Horizontal position of the object in pixels. This is in screen coordinates.
    int y;     ///< Vertical position of the object in pixels. This is in screen coordinates.
    int w;     ///< Width of the object in pixels.
    int h;     ///< Height of the object in pixels.
    void *obj; ///< Specialization object pointer, eg. label, button or menu struct.
    int id;    ///< Lookup ID. Default is -1, which means "not set". User should always set positive values!

    vector children; ///< Child components. Empty in the widget role; nothing is allocated until the first attach.

    int x_hint; ///< X position hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int y_hint; ///< Y position hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int w_hint; ///< W size hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int h_hint; ///< H size hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int w_required; ///< Width the content needs to render properly. -1 = not set.
    int h_required; ///< Height the content needs to render properly. -1 = not set.

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

    component *parent; ///< Parent component. NULL for root component.
};

void component_tick(component *c) {
    if(c->tick) {
        c->tick(c);
    }
    iterator it;
    component **child;
    vector_iter_begin(&c->children, &it);
    foreach(it, child) {
        component_tick(*child);
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
    iterator it;
    component **child;
    vector_iter_begin(&c->children, &it);
    foreach(it, child) {
        component_init(*child, theme);
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

vec2i component_get_required_size(const component *c) {
    return vec2i_create(c->w_required, c->h_required);
}

void component_set_required_size(component *c, int w, int h) {
    c->w_required = w;
    c->h_required = h;
}

vec2i component_get_preferred_size(const component *c) {
    return vec2i_create(c->w_hint >= 0 ? c->w_hint : c->w_required, c->h_hint >= 0 ? c->h_hint : c->h_required);
}

void component_set_pos_hints(component *c, int x, int y) {
    c->x_hint = x;
    c->y_hint = y;
}

component *component_find(component *c, int id) {
    if(c == NULL || id < 0) {
        return NULL;
    }
    if(c->id == id) {
        return c;
    }
    iterator it;
    component **child;
    vector_iter_begin(&c->children, &it);
    foreach(it, child) {
        component *out = component_find(*child, id);
        if(out != NULL) {
            return out;
        }
    }
    if(c->find) {
        return c->find(c, id);
    }
    return NULL;
}

void component_set_obj(component *c, void *obj) {
    assert(c->obj == NULL && "Specialization object is already set");
    c->obj = obj;
}

void *component_get_obj(const component *c) {
    return c->obj;
}

void component_set_id(component *c, int id) {
    assert(id >= 0 && "Component ID must be non-negative");
    c->id = id;
}

int component_get_id(const component *c) {
    return c->id;
}

void component_attach(component *c, component *nc) {
    assert(nc != NULL);
    component_set_parent(nc, c);
    vector_append(&c->children, &nc);
}

component *component_get_child(const component *c, int item) {
    component **child = vector_get(&c->children, item);
    if(child != NULL) {
        return *child;
    }
    return NULL;
}

int component_child_count(const component *c) {
    return vector_size(&c->children);
}

void component_iter_begin(const component *c, iterator *it) {
    vector_iter_begin(&c->children, it);
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
    assert((parent == NULL || c->parent == NULL) && "Component already has a parent");
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

component *component_create(void) {
    component *c = omf_calloc(1, sizeof(component));
    c->id = -1;
    // Zero capacity: widgets never allocate child storage.
    vector_create_with_size(&c->children, sizeof(component *), 0);
    component_set_supported(c, true, true, true);
    c->x_hint = -1;
    c->y_hint = -1;
    c->w_hint = -1;
    c->h_hint = -1;
    c->w_required = -1;
    c->h_required = -1;
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
    iterator it;
    component **child;
    vector_iter_begin(&c->children, &it);
    foreach(it, child) {
        component_free(*child);
    }
    if(c->free != NULL) {
        c->free(c);
    }
    vector_free(&c->children);
    if(c->help != NULL) {
        text_free(&c->help);
    }
    omf_free(c);
}
