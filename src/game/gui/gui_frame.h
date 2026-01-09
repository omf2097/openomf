/*! @file
 * @brief Top-level GUI frame that manages component hierarchy and events.
 */

#ifndef FRAME_H
#define FRAME_H

#include "game/gui/component.h"
#include "game/gui/theme.h"

typedef struct gui_frame gui_frame;

/** @brief Create a GUI frame with the specified theme and dimensions. */
gui_frame *gui_frame_create(const gui_theme *theme, int x, int y, int w, int h);

/** @brief Set the root component of the frame. */
void gui_frame_set_root(gui_frame *frame, component *root_node);

/** @brief Get the root component of the frame. */
component *gui_frame_get_root(const gui_frame *frame);

/** @brief Free the frame and its components. */
void gui_frame_free(gui_frame *frame);

/** @brief Find a component by ID within the frame. */
component *gui_frame_find(gui_frame *frame, int id);

/** @brief Get the frame position and dimensions. */
void gui_frame_get_measurements(const gui_frame *frame, int *x, int *y, int *w, int *h);

/** @brief Process one frame tick for all components. */
void gui_frame_tick(gui_frame *frame);

/** @brief Render all components in the frame. */
void gui_frame_render(gui_frame *frame);

/** @brief Handle an SDL event. */
int gui_frame_event(gui_frame *frame, SDL_Event *event);

/** @brief Handle a game action input. */
int gui_frame_action(gui_frame *frame, int action);

/** @brief Recalculate layout for all components. */
void gui_frame_layout(gui_frame *frame);

#endif // FRAME_H
