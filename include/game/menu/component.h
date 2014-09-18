#ifndef _COMPONENT_H
#define _COMPONENT_H

#include <SDL2/SDL.h>
#include "controller/controller.h"

enum {
    COM_ENABLED = 0,
    COM_DISABLED = 1,
};

enum {
    COM_UNSELECTED = 0,
    COM_SELECTED = 1,
};

typedef struct component_t component;

typedef void (*component_render_cb)(component *c);
typedef int (*component_event_cb)(component *c, SDL_Event *event);
typedef int (*component_action_cb)(component *c, int action);
typedef void (*component_layout_cb)(component *c, int x, int y, int w, int h);
typedef void (*component_tick_cb)(component *c);
typedef void (*component_free_cb)(component *c);

/*
* This is the basic component that you get by creating any textbutton, togglebutton, etc.
* The point is to abstract away rendering and event handling
*/
struct component_t {
    int x,y,w,h;
    void *obj;

    char supports_select;
    char is_selected;

    char supports_disable;
    char is_disabled;

    char supports_focus;
    char is_focused;

    component_render_cb render;
    component_event_cb event;
    component_action_cb action;
    component_layout_cb layout;
    component_tick_cb tick;
    component_free_cb free;

    component *parent;
};

// Create & free
component* component_create();
void component_free(component *c);

// Internal callbacks
void component_tick(component *c);
void component_render(component *c);
int component_event(component *c, SDL_Event *event);
int component_action(component *c, int action);
void component_layout(component *c, int x, int y, int w, int h);

void component_disable(component *c, int disabled);
void component_select(component *c, int selected);
void component_focus(component *c, int focused);
int component_is_disabled(const component *c);
int component_is_selected(const component *c);
int component_is_focused(const component *c);

// Basic component callbacks
void component_set_obj(component *c, void *obj);
void* component_get_obj(const component *c);
void component_set_render_cb(component *c, component_render_cb cb);
void component_set_event_cb(component *c, component_event_cb cb);
void component_set_action_cb(component *c, component_action_cb cb);
void component_set_layout_cb(component *c, component_layout_cb cb);
void component_set_tick_cb(component *c, component_tick_cb cb);
void component_set_free_cb(component *c, component_free_cb cb);

#endif // _COMPONENT_H
