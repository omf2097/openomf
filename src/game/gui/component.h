/*! \file
 * \brief Gui base component
 * \details Base component for GUI elements. Sizers, widgets etc. are based on this.
 * \copyright MIT license.
 * \date 2014
 * \author Tuomas Virtanen
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include "controller/controller.h"
#include <SDL.h>

enum
{
    COM_ENABLED = 0,  ///< Component enabled. Component is colored and can be interacted with.
    COM_DISABLED = 1, ///< Component disabled. Component is grayed out, and cannot be interacted with.
};

enum
{
    COM_UNSELECTED = 0, ///< Componen unselected. Used in eg. menu sizers.
    COM_SELECTED = 1,   ///< Component selected. Used in eg. menu sizers.
};

typedef struct component_t component;

typedef void (*component_render_cb)(component *c);
typedef int (*component_event_cb)(component *c, SDL_Event *event);
typedef int (*component_action_cb)(component *c, int action);
typedef void (*component_focus_cb)(component *c, bool focused);
typedef void (*component_layout_cb)(component *c, int x, int y, int w, int h);
typedef void (*component_tick_cb)(component *c);
typedef void (*component_free_cb)(component *c);
typedef component *(*component_find_cb)(component *c, int id);

/*! \brief Basic GUI object
 *
 * This is the basic component that you get by creating any textbutton, togglebutton, etc.
 * The point is to abstract away rendering and event handling.
 *
 * Note that the component doesn't have position or size before component_layout has been called.
 * Component_layout call for a sizer will cause all its children widgets and  sizers to be set also.
 */
struct component_t {
    int x;     ///< Horizontal position of the object in pixels. This is in screen coordinates.
    int y;     ///< Vertical position of the object in pixels. This is in screen coordinates.
    int w;     ///< Width of the object in pixels.
    int h;     ///< Height of the object in pixels.
    void *obj; ///< Specialization object pointer. Basically always Sizer or Widget struct.

    int x_hint; ///< X position hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int y_hint; ///< Y position hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int w_hint; ///< W size hint. Sizers may or may not obey this. -1 = not set. >=0 means set.
    int h_hint; ///< H size hint. Sizers may or may not obey this. -1 = not set. >=0 means set.

    char supports_select; ///< Whether the component can be selected by component_select() call.
    char is_selected;     ///< Whether the component is selected

    char supports_disable; ///< Whether the component can be disabled by component_disable() call.
    char is_disabled;      ///< Whether the component is disabled

    char supports_focus; ///< Whether the component can be focused by component_focus() call.
    char is_focused;     ///< Whether the component is focused
    const char *help;    ///< Help text, if available
    bool filler;         ///< Whether the component should fill unused space during layout

    component_render_cb render; ///< Render function callback. This tells the component to draw itself.
    component_event_cb event;   ///< Event function callback. Direct SDL2 event handler.
    component_action_cb action; ///< Action function callback. Handles OpenOMF abstract key events.
    component_focus_cb focus;   ///< Focus function callback. Handles OpenOMF focus events.
    component_layout_cb layout; ///< Layout function callback. This is called after the component tree is created. Sets
                                ///< component size and position.
    component_tick_cb tick;     ///< Tick function callback. This is called periodically.
    component_free_cb free;     ///< Free function callback. Any component callbacks should be done here.
    component_find_cb find;     ///< Should only be set by widget and sizer. Used to look up widgets by ID.

    component
        *parent; ///< Parent component. For widgets, this should be always a sizer. For root sizer it will be NULL.
};

// Create & free
component *component_create(void);
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

bool component_is_selectable(component *c);

void component_set_size_hints(component *c, int w, int h);
void component_set_pos_hints(component *c, int x, int y);

void component_set_help_text(component *c, const char *help);

// ID lookup stuff
component *component_find(component *c, int id);

// Basic component callbacks
void component_set_obj(component *c, void *obj);
void *component_get_obj(const component *c);
void component_set_render_cb(component *c, component_render_cb cb);
void component_set_event_cb(component *c, component_event_cb cb);
void component_set_action_cb(component *c, component_action_cb cb);
void component_set_focus_cb(component *c, component_focus_cb cb);
void component_set_layout_cb(component *c, component_layout_cb cb);
void component_set_tick_cb(component *c, component_tick_cb cb);
void component_set_free_cb(component *c, component_free_cb cb);
void component_set_find_cb(component *c, component_find_cb cb);

#endif // COMPONENT_H
