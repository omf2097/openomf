/*! @file
 * @brief Base widget structure for all leaf-node GUI elements.
 */

#ifndef WIDGET_H
#define WIDGET_H

#include "game/gui/component.h"

/** @brief Widget render callback type. */
typedef void (*widget_render_cb)(component *c);

/** @brief Widget SDL event callback type. Returns 0 if handled, 1 otherwise. */
typedef int (*widget_event_cb)(component *c, SDL_Event *event);

/** @brief Widget action callback type. Returns 0 if handled, 1 otherwise. */
typedef int (*widget_action_cb)(component *c, int action);

/** @brief Widget focus change callback type. */
typedef void (*widget_focus_cb)(component *c, bool focused);

/** @brief Widget layout callback type. Sets position and size. */
typedef void (*widget_layout_cb)(component *c, int x, int y, int w, int h);

/** @brief Widget per-frame tick callback type. */
typedef void (*widget_tick_cb)(component *c);

/** @brief Widget initialization callback type. Called before layout. */
typedef void (*widget_init_cb)(component *c, const gui_theme *theme);

/** @brief Widget cleanup callback type. */
typedef void (*widget_free_cb)(component *c);

/** @brief Create a new widget component. */
component *widget_create(void);

/** @brief Set the specialization object pointer. */
void widget_set_obj(component *c, void *obj);

/** @brief Get the specialization object pointer. */
void *widget_get_obj(const component *c);

/** @brief Set the widget ID for lookup via gui_frame_find(). */
void widget_set_id(component *c, int id);

/** @brief Get the widget ID. */
int widget_get_id(const component *c);

/** @brief Set the render callback. */
void widget_set_render_cb(component *c, widget_render_cb cb);

/** @brief Set the SDL event callback. */
void widget_set_event_cb(component *c, widget_event_cb cb);

/** @brief Set the action callback. */
void widget_set_action_cb(component *c, widget_action_cb cb);

/** @brief Set the focus change callback. */
void widget_set_focus_cb(component *c, widget_focus_cb cb);

/** @brief Set the layout callback. */
void widget_set_layout_cb(component *c, widget_layout_cb cb);

/** @brief Set the tick callback. */
void widget_set_tick_cb(component *c, widget_tick_cb cb);

/** @brief Set the initialization callback. */
void widget_set_init_cb(component *c, widget_init_cb cb);

/** @brief Set the cleanup callback. */
void widget_set_free_cb(component *c, widget_free_cb cb);

#endif // WIDGET_H
