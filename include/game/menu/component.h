#ifndef _COMPONENT_H
#define _COMPONENT_H

#include <SDL2/SDL.h>
#include "controller/controller.h"

typedef struct component_t component;

typedef void (*component_render_cb)(component *c);
typedef int (*component_event_cb)(component *c, SDL_Event *event);
typedef int (*component_action_cb)(component *c, int action);
typedef void (*component_layout_cb)(component *c, int x, int y, int w, int h);
typedef void (*component_tick_cb)(component *c);

typedef void (*component_click_cb)(component *c, void *userdata);
typedef void (*component_toggle_cb)(component *c, void *userdata, int option);
typedef void (*component_slide_cb)(component *c, void *userdata, int pos);
typedef void (*component_focus_cb)(component *c, void *userdata, int focus);

/*
* This is the basic component that you get by creating any textbutton, togglebutton, etc.
* The point is to abstract away rendering and event handling
*/
struct component_t {
    int x,y,w,h;
    int id;
    // could use a bitmask for the different states
    int selected;
    int disabled;
    void *obj;

    component_render_cb render;
    component_event_cb event;
    component_action_cb action;
    component_layout_cb layout;
    component_tick_cb tick;

    void *cb_userdata;
    component_click_cb on_click;
    component_toggle_cb on_toggle;
    component_slide_cb on_slide;
    component_focus_cb on_focus;
};

// Create & free
void component_create(component *c);
void component_free(component *c);

// Make the component do something
void component_click(component *c);
void component_toggle(component *c, int option);
void component_slide(component *c, int pos);
void component_focus(component *c, int focus);

// Behaviour stuff
void component_disable(component *c, int disabled);
void component_select(component *c, int selected);

// Internal callbacks
void component_tick(component *c);
void component_render(component *c);
int component_event(component *c, SDL_Event *event);
int component_action(component *c, int action);
void component_layout(component *c, int x, int y, int w, int h);

// Basic component callbacks
void component_set_obj(component *c, void *obj);
void component_set_render_cb(component *c, component_render_cb cb);
void component_set_event_cb(component *c, component_event_cb cb);
void component_set_action_cb(component *c, component_action_cb cb);
void component_set_layout_cb(component *c, component_layout_cb cb);
void component_set_tick_cb(component *c, component_tick_cb cb);

// Event callback functions
void component_set_onclick(component *c, component_click_cb cb, void *userdata);
void component_set_ontoggle(component *c, component_toggle_cb cb, void *userdata);
void component_set_onslide(component *c, component_slide_cb cb, void *userdata);
void component_set_onfocus(component *c, component_focus_cb cb, void *userdata);

// ID stuff (for use outside of component)
void component_set_id(component *c, int id);
int component_get_id(const component *c);

#endif // _COMPONENT_H
