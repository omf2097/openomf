#ifndef WIDGET_H
#define WIDGET_H

#include "game/gui/component.h"

typedef void (*widget_render_cb)(component *c);
typedef int (*widget_event_cb)(component *c, SDL_Event *event);
typedef int (*widget_action_cb)(component *c, int action);
typedef void (*widget_focus_cb)(component *c, bool focused);
typedef void (*widget_layout_cb)(component *c, int x, int y, int w, int h);
typedef void (*widget_tick_cb)(component *c);
typedef void (*widget_free_cb)(component *c);

typedef struct {
    void *obj; ///< Pointer to internal object, eg. textbutton, etc.
    int id;    ///< Default is -1, which means "not set". User should always set positive values!

    widget_render_cb render;
    widget_event_cb event;
    widget_action_cb action;
    widget_focus_cb focus;
    widget_layout_cb layout;
    widget_tick_cb tick;
    widget_free_cb free;
} widget;

component *widget_create(void);

void widget_set_obj(component *c, void *obj);
void *widget_get_obj(const component *c);

void widget_set_id(component *c, int id);
int widget_get_id(const component *c);

void widget_set_render_cb(component *c, widget_render_cb cb);
void widget_set_event_cb(component *c, widget_event_cb cb);
void widget_set_action_cb(component *c, widget_action_cb cb);
void widget_set_focus_cb(component *c, widget_focus_cb cb);
void widget_set_layout_cb(component *c, widget_layout_cb cb);
void widget_set_tick_cb(component *c, widget_tick_cb cb);
void widget_set_free_cb(component *c, widget_free_cb cb);

#endif // WIDGET_H
