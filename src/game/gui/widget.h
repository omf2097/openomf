/**
 * @file widget.h
 * @brief GUI widget base component
 * @details Base widget component that provides common functionality for all widgets.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef WIDGET_H
#define WIDGET_H

#include "game/gui/component.h"

typedef void (*widget_render_cb)(component *c);                             ///< Widget render callback
typedef int (*widget_event_cb)(component *c, SDL_Event *event);             ///< Widget SDL event callback
typedef int (*widget_action_cb)(component *c, int action);                  ///< Widget action callback
typedef void (*widget_focus_cb)(component *c, bool focused);                ///< Widget focus change callback
typedef void (*widget_layout_cb)(component *c, int x, int y, int w, int h); ///< Widget layout callback
typedef void (*widget_tick_cb)(component *c);                               ///< Widget tick callback
typedef void (*widget_init_cb)(component *c, const gui_theme *theme);       ///< Widget initialization callback
typedef void (*widget_free_cb)(component *c);                               ///< Widget free callback

/**
 * @brief Create a base widget
 * @return Pointer to the newly created widget component
 */
component *widget_create(void);

/**
 * @brief Set the widget's specialization object
 * @param c Widget component to modify
 * @param obj Object pointer to store
 */
void widget_set_obj(component *c, void *obj);

/**
 * @brief Get the widget's specialization object
 * @param c Widget component to query
 * @return Stored object pointer
 */
void *widget_get_obj(const component *c);

/**
 * @brief Set the widget's ID for lookup
 * @param c Widget component to modify
 * @param id Widget ID
 */
void widget_set_id(component *c, int id);

/**
 * @brief Get the widget's ID
 * @param c Widget component to query
 * @return Widget ID
 */
int widget_get_id(const component *c);

/**
 * @brief Set the render callback
 * @param c Widget component to modify
 * @param cb Render callback function
 */
void widget_set_render_cb(component *c, widget_render_cb cb);

/**
 * @brief Set the event callback
 * @param c Widget component to modify
 * @param cb Event callback function
 */
void widget_set_event_cb(component *c, widget_event_cb cb);

/**
 * @brief Set the action callback
 * @param c Widget component to modify
 * @param cb Action callback function
 */
void widget_set_action_cb(component *c, widget_action_cb cb);

/**
 * @brief Set the focus callback
 * @param c Widget component to modify
 * @param cb Focus callback function
 */
void widget_set_focus_cb(component *c, widget_focus_cb cb);

/**
 * @brief Set the layout callback
 * @param c Widget component to modify
 * @param cb Layout callback function
 */
void widget_set_layout_cb(component *c, widget_layout_cb cb);

/**
 * @brief Set the tick callback
 * @param c Widget component to modify
 * @param cb Tick callback function
 */
void widget_set_tick_cb(component *c, widget_tick_cb cb);

/**
 * @brief Set the initialization callback
 * @param c Widget component to modify
 * @param cb Initialization callback function
 */
void widget_set_init_cb(component *c, widget_init_cb cb);

/**
 * @brief Set the free callback
 * @param c Widget component to modify
 * @param cb Free callback function
 */
void widget_set_free_cb(component *c, widget_free_cb cb);

#endif // WIDGET_H
