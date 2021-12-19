#ifndef SIZER_H
#define SIZER_H

#include "game/gui/component.h"
#include "utils/vector.h"

typedef void (*sizer_render_cb)(component *c);
typedef int (*sizer_event_cb)(component *c, SDL_Event *event);
typedef int (*sizer_action_cb)(component *c, int action);
typedef void (*sizer_layout_cb)(component *c, int x, int y, int w, int h);
typedef void (*sizer_tick_cb)(component *c);
typedef void (*sizer_free_cb)(component *c);
typedef component* (*sizer_find_cb)(component *c, int id);

typedef struct {
    void *obj; ///< Sizer specialization object, eg. menu, trnmenu
    vector objs; ///< Contains all the child objects in the sizer

    float opacity; ///< Some sizers may want to fade their contents (eg. tournament menu). In these cases, if should be handled via this variable.

    sizer_render_cb render;
    sizer_event_cb event;
    sizer_action_cb action;
    sizer_layout_cb layout;
    sizer_tick_cb tick;
    sizer_free_cb free;
    sizer_find_cb find;
} sizer;

component* sizer_create();

void sizer_set_obj(component *c, void *obj);
void* sizer_get_obj(const component *c);

component* sizer_get(const component *c, int item);
int sizer_size(const component *c);

void sizer_set_render_cb(component *c, sizer_render_cb cb);
void sizer_set_event_cb(component *c, sizer_event_cb cb);
void sizer_set_action_cb(component *c, sizer_action_cb cb);
void sizer_set_layout_cb(component *c, sizer_layout_cb cb);
void sizer_set_tick_cb(component *c, sizer_tick_cb cb);
void sizer_set_free_cb(component *c, sizer_free_cb cb);
void sizer_set_find_cb(component *c, sizer_find_cb cb);

void sizer_attach(component *c, component *nc);

#endif // SIZER_H
