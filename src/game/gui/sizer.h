/*! @file
 * @brief Base container structure for all sizer components.
 */

#ifndef SIZER_H
#define SIZER_H

#include "game/gui/component.h"
#include "utils/vector.h"

/** @brief Sizer render callback type. */
typedef void (*sizer_render_cb)(component *c);

/** @brief Sizer SDL event callback type. Returns 0 if handled, 1 otherwise. */
typedef int (*sizer_event_cb)(component *c, SDL_Event *event);

/** @brief Sizer action callback type. Returns 0 if handled, 1 otherwise. */
typedef int (*sizer_action_cb)(component *c, int action);

/** @brief Sizer layout callback type. Sets position and size. */
typedef void (*sizer_layout_cb)(component *c, int x, int y, int w, int h);

/** @brief Sizer per-frame tick callback type. */
typedef void (*sizer_tick_cb)(component *c);

/** @brief Sizer initialization callback type. Called before layout. */
typedef void (*sizer_init_cb)(component *c, const gui_theme *theme);

/** @brief Sizer cleanup callback type. */
typedef void (*sizer_free_cb)(component *c);

/** @brief Sizer child lookup callback type. Returns component if ID matches, NULL otherwise. */
typedef component *(*sizer_find_cb)(component *c, int id);

typedef struct sizer sizer;

/** @brief Create a new sizer. */
component *sizer_create(void);

/** @brief Set the specialized object pointer. */
void sizer_set_obj(component *c, void *obj);

/** @brief Get the specialized object pointer. */
void *sizer_get_obj(const component *c);

/** @brief Get child by index. */
component *sizer_get(const component *c, int item);

/** @brief Get the number of children. */
int sizer_size(const component *c);

/** @brief Get the current opacity. */
float sizer_get_opacity(const component *c);

/** @brief Set the opacity for fade effects. */
void sizer_set_opacity(const component *c, float opacity);

/** @brief Start iterating children. */
void sizer_begin_iterator(const component *c, iterator *it);

/** @brief Set the render callback. */
void sizer_set_render_cb(component *c, sizer_render_cb cb);

/** @brief Set the event callback. */
void sizer_set_event_cb(component *c, sizer_event_cb cb);

/** @brief Set the action callback. */
void sizer_set_action_cb(component *c, sizer_action_cb cb);

/** @brief Set the layout callback. */
void sizer_set_layout_cb(component *c, sizer_layout_cb cb);

/** @brief Set the tick callback. */
void sizer_set_tick_cb(component *c, sizer_tick_cb cb);

/** @brief Set the free callback. */
void sizer_set_free_cb(component *c, sizer_free_cb cb);

/** @brief Set the init callback. */
void sizer_set_init_cb(component *c, sizer_init_cb cb);

/** @brief Set the find callback. */
void sizer_set_find_cb(component *c, sizer_find_cb cb);

/** @brief Add a child component to the sizer. */
void sizer_attach(component *c, component *nc);

#endif // SIZER_H
