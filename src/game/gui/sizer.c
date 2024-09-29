#include "game/gui/sizer.h"
#include "utils/allocator.h"

component *sizer_get(const component *nc, int item) {
    sizer *local = component_get_obj(nc);
    component **c;
    c = vector_get(&local->objs, item);
    if(c != NULL) {
        return *c;
    }
    return NULL;
}

int sizer_size(const component *c) {
    sizer *local = component_get_obj(c);
    return vector_size(&local->objs);
}

void sizer_set_obj(component *c, void *obj) {
    sizer *local = component_get_obj(c);
    local->obj = obj;
}

void *sizer_get_obj(const component *c) {
    sizer *local = component_get_obj(c);
    return local->obj;
}

void sizer_set_render_cb(component *c, sizer_render_cb cb) {
    sizer *local = component_get_obj(c);
    local->render = cb;
}

void sizer_set_event_cb(component *c, sizer_event_cb cb) {
    sizer *local = component_get_obj(c);
    local->event = cb;
}

void sizer_set_action_cb(component *c, sizer_action_cb cb) {
    sizer *local = component_get_obj(c);
    local->action = cb;
}

void sizer_set_layout_cb(component *c, sizer_layout_cb cb) {
    sizer *local = component_get_obj(c);
    local->layout = cb;
}

void sizer_set_tick_cb(component *c, sizer_tick_cb cb) {
    sizer *local = component_get_obj(c);
    local->tick = cb;
}

void sizer_set_free_cb(component *c, sizer_free_cb cb) {
    sizer *local = component_get_obj(c);
    local->free = cb;
}

void sizer_set_find_cb(component *c, sizer_find_cb cb) {
    sizer *local = component_get_obj(c);
    local->find = cb;
}

void sizer_attach(component *c, component *nc) {
    sizer *local = component_get_obj(c);
    nc->parent = c;
    vector_append(&local->objs, &nc);
}

static void sizer_tick(component *c) {
    sizer *local = component_get_obj(c);

    // Tell the specialized sizer object to do tick if needed
    if(local->tick) {
        local->tick(c);
    }

    // Tick all components in sizer.
    iterator it;
    component **tmp;
    vector_iter_begin(&local->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        component_tick(*tmp);
    }
}

static void sizer_render(component *c) {
    sizer *local = component_get_obj(c);
    // Since rendering can be a bit special, the actual sizer should do it
    if(local->render) {
        local->render(c);
    }
}

static int sizer_event(component *c, SDL_Event *event) {
    sizer *local = component_get_obj(c);
    // Events are something that the actual sizer needs to handle
    if(local->event) {
        return local->event(c, event);
    }
    return 1;
}

static int sizer_action(component *c, int action) {
    sizer *local = component_get_obj(c);
    // Actions are something that the actual sizer needs to handle
    if(local->action) {
        return local->action(c, action);
    }
    return 1;
}

static void sizer_layout(component *c, int x, int y, int w, int h) {
    // Because we don't know how to order this stuff in base sizer, we just pass this on.
    sizer *local = component_get_obj(c);
    if(local->layout) {
        local->layout(c, x, y, w, h);
    }
}

static void sizer_free(component *c) {
    sizer *local = component_get_obj(c);

    // Free all objects inside the sizer
    iterator it;
    component **tmp;
    vector_iter_begin(&local->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        component_free(*tmp);
    }

    // Free sizer specialization
    if(local->free) {
        local->free(c);
    }

    // Free sizer itself
    vector_free(&local->objs);
    omf_free(local);
}

static component *sizer_find(component *c, int id) {
    sizer *local = component_get_obj(c);

    iterator it;
    component **tmp;
    vector_iter_begin(&local->objs, &it);
    while((tmp = iter_next(&it)) != NULL) {
        // Find out if the component is what we're looking for.
        // If it is, return pointer.
        component *out = component_find(*tmp, id);
        if(out != NULL) {
            return out;
        }
    }

    // If find callback is set, try to use it.
    if(local->find) {
        return local->find(c, id);
    }

    // If requested ID was not found in any of the internal components/widgets, return NULL.
    return NULL;
}

component *sizer_create(void) {
    component *c = component_create();

    sizer *local = omf_calloc(1, sizeof(sizer));
    vector_create(&local->objs, sizeof(component *));
    component_set_obj(c, local);

    component_set_tick_cb(c, sizer_tick);
    component_set_render_cb(c, sizer_render);
    component_set_event_cb(c, sizer_event);
    component_set_action_cb(c, sizer_action);
    component_set_layout_cb(c, sizer_layout);
    component_set_free_cb(c, sizer_free);
    component_set_find_cb(c, sizer_find);

    return c;
}
