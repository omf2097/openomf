/**
 * @file component.h
 * @brief GUI base component
 * @details Base component for GUI elements. Sizers, widgets etc. are based on this.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include "controller/controller.h"
#include "game/gui/text/text.h"
#include "game/gui/theme.h"
#include <SDL.h>

typedef struct component component;

typedef void (*component_render_cb)(component *c);                             ///< Render callback function type
typedef int (*component_event_cb)(component *c, SDL_Event *event);             ///< SDL event callback function type
typedef int (*component_action_cb)(component *c, int action);                  ///< Action callback function type
typedef void (*component_focus_cb)(component *c, bool focused);                ///< Focus change callback function type
typedef void (*component_layout_cb)(component *c, int x, int y, int w, int h); ///< Layout callback function type
typedef void (*component_tick_cb)(component *c);                               ///< Tick/update callback function type
typedef void (*component_free_cb)(component *c);                               ///< Free/cleanup callback function type
typedef void (*component_init_cb)(component *c, const gui_theme *theme); ///< Initialization callback function type
typedef component *(*component_find_cb)(component *c, int id); ///< Find component by ID callback function type

/**
 * @brief Basic GUI object
 *
 * This is the basic component that you get by creating any textbutton, togglebutton, etc.
 * The point is to abstract away rendering and event handling.
 *
 * @note The component doesn't have position or size before component_layout has been called.
 * Component_layout call for a sizer will cause all its children widgets and sizers to be set also.
 */
struct component {
    uint32_t header; ///< Safety header. 0xDEADBEEF for sizers, 0x8BADF00D for components.

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

/**
 * @brief Create a new component
 * @param header Safety header value (0xDEADBEEF for sizers, 0x8BADF00D for widgets)
 * @return Pointer to the newly created component
 */
component *component_create(uint32_t header);

/**
 * @brief Free a component and its resources
 * @param c Component to free
 */
void component_free(component *c);

/**
 * @brief Process a tick update for the component
 * @param c Component to tick
 */
void component_tick(component *c);

/**
 * @brief Render the component
 * @param c Component to render
 */
void component_render(component *c);

/**
 * @brief Handle an SDL event
 * @param c Component to receive the event
 * @param event SDL event to process
 * @return Non-zero if the event was handled
 */
int component_event(component *c, SDL_Event *event);

/**
 * @brief Handle an abstract action event
 * @param c Component to receive the action
 * @param action Action code to process
 * @return Non-zero if the action was handled
 */
int component_action(component *c, int action);

/**
 * @brief Initialize the component with a theme
 * @param c Component to initialize
 * @param theme Theme to apply
 */
void component_init(component *c, const gui_theme *theme);

/**
 * @brief Set the layout (position and size) of the component
 * @param c Component to layout
 * @param x X coordinate in screen pixels
 * @param y Y coordinate in screen pixels
 * @param w Width in pixels
 * @param h Height in pixels
 */
void component_layout(component *c, int x, int y, int w, int h);

/**
 * @brief Set the disabled state of the component
 * @param c Component to modify
 * @param disabled True to disable, false to enable
 */
void component_disable(component *c, bool disabled);

/**
 * @brief Set the selected state of the component
 * @param c Component to modify
 * @param selected True to select, false to deselect
 */
void component_select(component *c, bool selected);

/**
 * @brief Set the focused state of the component
 * @param c Component to modify
 * @param focused True to focus, false to unfocus
 */
void component_focus(component *c, bool focused);

/**
 * @brief Check if the component is disabled
 * @param c Component to check
 * @return True if disabled
 */
bool component_is_disabled(const component *c);

/**
 * @brief Check if the component is selected
 * @param c Component to check
 * @return True if selected
 */
bool component_is_selected(const component *c);

/**
 * @brief Check if the component is focused
 * @param c Component to check
 * @return True if focused
 */
bool component_is_focused(const component *c);

/**
 * @brief Check if the component can be selected
 * @param c Component to check
 * @return True if the component supports selection and is not disabled
 */
bool component_is_selectable(component *c);

/**
 * @brief Set size hints for the component
 * @param c Component to modify
 * @param w Width hint (-1 for not set)
 * @param h Height hint (-1 for not set)
 */
void component_set_size_hints(component *c, int w, int h);

/**
 * @brief Set position hints for the component
 * @param c Component to modify
 * @param x X position hint (-1 for not set)
 * @param y Y position hint (-1 for not set)
 */
void component_set_pos_hints(component *c, int x, int y);

/**
 * @brief Set which features the component supports
 * @param c Component to modify
 * @param allow_disable Whether the component can be disabled
 * @param allow_select Whether the component can be selected
 * @param allow_focus Whether the component can be focused
 */
void component_set_supports(component *c, bool allow_disable, bool allow_select, bool allow_focus);

/**
 * @brief Set help text for the component
 * @param c Component to modify
 * @param text Help text string
 */
void component_set_help_text(component *c, const char *text);

/**
 * @brief Set the theme for the component
 * @param c Component to modify
 * @param theme Theme to set
 */
void component_set_theme(component *c, const gui_theme *theme);

/**
 * @brief Get the theme from the component
 * @param c Component to query
 * @return Pointer to the component's theme
 */
const gui_theme *component_get_theme(component *c);

/**
 * @brief Find a component by ID within a component tree
 * @param c Root component to search from
 * @param id ID to search for
 * @return Pointer to the found component, or NULL if not found
 */
component *component_find(component *c, int id);

/**
 * @brief Set the specialization object for the component
 * @param c Component to modify
 * @param obj Object pointer (typically sizer or widget struct)
 */
void component_set_obj(component *c, void *obj);

/**
 * @brief Get the specialization object from the component
 * @param c Component to query
 * @return Object pointer
 */
void *component_get_obj(const component *c);

/**
 * @brief Set the render callback
 * @param c Component to modify
 * @param cb Render callback function
 */
void component_set_render_cb(component *c, component_render_cb cb);

/**
 * @brief Set the event callback
 * @param c Component to modify
 * @param cb Event callback function
 */
void component_set_event_cb(component *c, component_event_cb cb);

/**
 * @brief Set the action callback
 * @param c Component to modify
 * @param cb Action callback function
 */
void component_set_action_cb(component *c, component_action_cb cb);

/**
 * @brief Set the focus callback
 * @param c Component to modify
 * @param cb Focus callback function
 */
void component_set_focus_cb(component *c, component_focus_cb cb);

/**
 * @brief Set the layout callback
 * @param c Component to modify
 * @param cb Layout callback function
 */
void component_set_layout_cb(component *c, component_layout_cb cb);

/**
 * @brief Set the initialization callback
 * @param c Component to modify
 * @param cb Initialization callback function
 */
void component_set_init_cb(component *c, component_init_cb cb);

/**
 * @brief Set the tick callback
 * @param c Component to modify
 * @param cb Tick callback function
 */
void component_set_tick_cb(component *c, component_tick_cb cb);

/**
 * @brief Set the free callback
 * @param c Component to modify
 * @param cb Free callback function
 */
void component_set_free_cb(component *c, component_free_cb cb);

/**
 * @brief Set the find callback
 * @param c Component to modify
 * @param cb Find callback function
 */
void component_set_find_cb(component *c, component_find_cb cb);

#endif // COMPONENT_H
