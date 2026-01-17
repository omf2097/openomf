/**
 * @file sizer.h
 * @brief GUI sizer base component
 * @details A base sizer component for arranging child components. Extended by specific sizer types.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SIZER_H
#define SIZER_H

#include "game/gui/component.h"

typedef void (*sizer_render_cb)(component *c);                             ///< Sizer render callback
typedef int (*sizer_event_cb)(component *c, SDL_Event *event);             ///< Sizer SDL event callback
typedef int (*sizer_action_cb)(component *c, int action);                  ///< Sizer action callback
typedef void (*sizer_layout_cb)(component *c, int x, int y, int w, int h); ///< Sizer layout callback
typedef void (*sizer_tick_cb)(component *c);                               ///< Sizer tick callback
typedef void (*sizer_init_cb)(component *c, const gui_theme *theme);       ///< Sizer initialization callback
typedef void (*sizer_free_cb)(component *c);                               ///< Sizer free callback
typedef component *(*sizer_find_cb)(component *c, int id);                 ///< Sizer find by ID callback

typedef struct sizer sizer;

/**
 * @brief Create a base sizer
 * @return Pointer to the newly created sizer component
 */
component *sizer_create(void);

/**
 * @brief Set the sizer's specialization object
 * @param c Sizer component to modify
 * @param obj Object pointer to store
 */
void sizer_set_obj(component *c, void *obj);

/**
 * @brief Get the sizer's specialization object
 * @param c Sizer component to query
 * @return Stored object pointer
 */
void *sizer_get_obj(const component *c);

/**
 * @brief Get a child component by index
 * @param c Sizer component to query
 * @param item Index of the child
 * @return Pointer to the child component
 */
component *sizer_get(const component *c, int item);

/**
 * @brief Get the number of children
 * @param c Sizer component to query
 * @return Number of child components
 */
int sizer_size(const component *c);

/**
 * @brief Get the sizer's opacity
 * @param c Sizer component to query
 * @return Opacity value (0.0 to 1.0)
 */
float sizer_get_opacity(const component *c);

/**
 * @brief Set the sizer's opacity
 * @param c Sizer component to modify
 * @param opacity Opacity value (0.0 to 1.0)
 */
void sizer_set_opacity(const component *c, float opacity);

/**
 * @brief Begin iterating over child components
 * @param c Sizer component to iterate
 * @param it Iterator to initialize
 */
void sizer_begin_iterator(const component *c, iterator *it);

/**
 * @brief Set the render callback
 * @param c Sizer component to modify
 * @param cb Render callback function
 */
void sizer_set_render_cb(component *c, sizer_render_cb cb);

/**
 * @brief Set the event callback
 * @param c Sizer component to modify
 * @param cb Event callback function
 */
void sizer_set_event_cb(component *c, sizer_event_cb cb);

/**
 * @brief Set the action callback
 * @param c Sizer component to modify
 * @param cb Action callback function
 */
void sizer_set_action_cb(component *c, sizer_action_cb cb);

/**
 * @brief Set the layout callback
 * @param c Sizer component to modify
 * @param cb Layout callback function
 */
void sizer_set_layout_cb(component *c, sizer_layout_cb cb);

/**
 * @brief Set the tick callback
 * @param c Sizer component to modify
 * @param cb Tick callback function
 */
void sizer_set_tick_cb(component *c, sizer_tick_cb cb);

/**
 * @brief Set the free callback
 * @param c Sizer component to modify
 * @param cb Free callback function
 */
void sizer_set_free_cb(component *c, sizer_free_cb cb);

/**
 * @brief Set the initialization callback
 * @param c Sizer component to modify
 * @param cb Initialization callback function
 */
void sizer_set_init_cb(component *c, sizer_init_cb cb);

/**
 * @brief Set the find callback
 * @param c Sizer component to modify
 * @param cb Find callback function
 */
void sizer_set_find_cb(component *c, sizer_find_cb cb);

/**
 * @brief Attach a child component to the sizer
 * @param c Sizer component to attach to
 * @param nc Child component to attach
 */
void sizer_attach(component *c, component *nc);

#endif // SIZER_H
