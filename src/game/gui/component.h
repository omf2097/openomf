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

#include "game/gui/text/text.h"
#include "game/gui/theme.h"
#include "utils/vec.h"
#include <SDL.h>

typedef struct component component;

typedef void (*component_render_cb)(component *c);                             ///< Render callback function type
typedef bool (*component_event_cb)(component *c, SDL_Event *event);            ///< SDL event callback function type
typedef bool (*component_action_cb)(component *c, int action, int source);     ///< Action callback function type
typedef void (*component_focus_cb)(component *c, bool focused);                ///< Focus change callback function type
typedef void (*component_layout_cb)(component *c, int x, int y, int w, int h); ///< Layout callback function type
typedef void (*component_tick_cb)(component *c);                               ///< Tick/update callback function type
typedef void (*component_free_cb)(component *c);                               ///< Free/cleanup callback function type
typedef void (*component_init_cb)(component *c, const gui_theme *theme); ///< Initialization callback function type
typedef component *(*component_find_cb)(component *c, int id); ///< Find component by ID callback function type

/**
 * @brief Component type
 */
typedef enum component_type
{
    COMPONENT_SIZER = 0x1337C0D3,  ///< Component is a sizer (contains children)
    COMPONENT_WIDGET = 0x1337BEEF, ///< Component is a widget (leaf element)
} component_type;

/**
 * @brief Create a new component
 *
 * This is the basic component that you get by creating any textbutton, togglebutton, etc.
 * The point is to abstract away rendering and event handling.
 *
 * @note The component doesn't have position or size before component_layout has been called.
 * Component_layout call for a sizer will cause all its children widgets and sizers to be set also.
 *
 * @param type Whether the component is a sizer or a widget
 * @return Pointer to the newly created component
 */
component *component_create(component_type type);

/**
 * @brief Check if the component is a sizer
 * @param c Component to check
 * @return True if the component is a sizer
 */
bool component_is_sizer(const component *c);

/**
 * @brief Check if the component is a widget
 * @param c Component to check
 * @return True if the component is a widget
 */
bool component_is_widget(const component *c);

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
 * @return True if the event was consumed, false if not
 */
bool component_event(component *c, SDL_Event *event);

/**
 * @brief Handle an abstract action event
 * @param c Component to receive the action
 * @param action Action code to process
 * @param source CTRL_TYPE_* of the device that produced the action
 * @return True if the action was consumed, false if not
 */
bool component_action(component *c, int action, int source);

/**
 * @brief Initialize the component with a theme
 * @param c Component to initialize
 * @param theme Theme to apply
 */
void component_init(component *c, const gui_theme *theme);

/**
 * @brief Get the position of the component in pixels
 * @param c Component to query
 * @return Position set by the last layout call
 */
vec2i component_get_pos(const component *c);

/**
 * @brief Get the size of the component in pixels
 * @param c Component to query
 * @return Size set by the last layout call
 */
vec2i component_get_size(const component *c);

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
 * @return True if the component supports selection
 */
bool component_is_selectable(component *c);

/**
 * @brief Sets whether the component can be selected
 * @param c Component to modify
 * @param selectable True to allow selection, false to disallow.
 */
void component_set_selectable(component *c, bool selectable);

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
 * @brief Get the position hints of the component
 * @param c Component to query
 * @return Position hint, -1 on an axis means not set
 */
vec2i component_get_pos_hint(const component *c);

/**
 * @brief Get the size hints of the component
 * @param c Component to query
 * @return Size hint, -1 on an axis means not set
 */
vec2i component_get_size_hint(const component *c);

/**
 * @brief Set which features the component supports
 * @param c Component to modify
 * @param allow_disable Whether the component can be disabled
 * @param allow_select Whether the component can be selected
 * @param allow_focus Whether the component can be focused
 */
void component_set_supported(component *c, bool allow_disable, bool allow_select, bool allow_focus);

/**
 * @brief Check if the component is in edit mode and owns input
 * @param c Component to check
 * @return True if editing
 */
bool component_is_editing(const component *c);

/**
 * @brief Set the edit mode of the component
 * @param c Component to modify
 * @param editing True to enter edit mode, false to leave it
 */
void component_set_editing(component *c, bool editing);

/**
 * @brief Check if render inputs have changed since the last render
 * @param c Component to check
 * @return True if dirty
 */
bool component_is_dirty(const component *c);

/**
 * @brief Set the dirty flag of the component
 * @param c Component to modify
 * @param dirty True to mark render inputs changed, false to clear the flag
 */
void component_set_dirty(component *c, bool dirty);

/**
 * @brief Get the parent component
 * @param c Component to query
 * @return Parent component, or NULL for the root component
 */
component *component_get_parent(const component *c);

/**
 * @brief Set the parent component
 * @param c Component to modify
 * @param parent Parent component, or NULL for the root component
 */
void component_set_parent(component *c, component *parent);

/**
 * @brief Set help text for the component
 * @param c Component to modify
 * @param text Help text string
 */
void component_set_help_text(component *c, const char *text);

/**
 * @brief Get the help text of the component
 * @param c Component to query
 * @return Help text object, or NULL if none has been set
 */
text *component_get_help_text(const component *c);

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
