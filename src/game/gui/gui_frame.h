/**
 * @file gui_frame.h
 * @brief GUI frame container
 * @details A frame that contains and manages a GUI component tree with layout, events, and rendering.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef FRAME_H
#define FRAME_H

#include "game/gui/component.h"
#include "game/gui/theme.h"

typedef struct gui_frame gui_frame;

/**
 * @brief Create a GUI frame
 * @param theme Theme to use for the frame's components
 * @param x X coordinate of the frame
 * @param y Y coordinate of the frame
 * @param w Width of the frame
 * @param h Height of the frame
 * @return Pointer to the newly created frame
 */
gui_frame *gui_frame_create(const gui_theme *theme, int x, int y, int w, int h);

/**
 * @brief Set the root component of the frame
 * @param frame Frame to modify
 * @param root_node Root component of the GUI tree
 */
void gui_frame_set_root(gui_frame *frame, component *root_node);

/**
 * @brief Get the root component of the frame
 * @param frame Frame to query
 * @return Pointer to the root component
 */
component *gui_frame_get_root(const gui_frame *frame);

/**
 * @brief Free the frame and its resources
 * @param frame Frame to free
 */
void gui_frame_free(gui_frame *frame);

/**
 * @brief Find a component by ID within the frame
 * @param frame Frame to search
 * @param id ID to search for
 * @return Pointer to the found component, or NULL if not found
 */
component *gui_frame_find(gui_frame *frame, int id);

/**
 * @brief Get the frame's measurements
 * @param frame Frame to query
 * @param x Output pointer for X coordinate
 * @param y Output pointer for Y coordinate
 * @param w Output pointer for width
 * @param h Output pointer for height
 */
void gui_frame_get_measurements(const gui_frame *frame, int *x, int *y, int *w, int *h);

/**
 * @brief Process a tick update for the frame
 * @param frame Frame to tick
 */
void gui_frame_tick(gui_frame *frame);

/**
 * @brief Render the frame and its contents
 * @param frame Frame to render
 */
void gui_frame_render(gui_frame *frame);

/**
 * @brief Handle an SDL event
 * @param frame Frame to receive the event
 * @param event SDL event to process
 * @return Non-zero if the event was handled
 */
int gui_frame_event(gui_frame *frame, SDL_Event *event);

/**
 * @brief Handle an abstract action event
 * @param frame Frame to receive the action
 * @param action Action code to process
 * @return Non-zero if the action was handled
 */
int gui_frame_action(gui_frame *frame, int action);

/**
 * @brief Perform layout calculation for the frame
 * @param frame Frame to layout
 */
void gui_frame_layout(gui_frame *frame);

#endif // FRAME_H
