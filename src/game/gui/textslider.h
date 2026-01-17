/**
 * @file textslider.h
 * @brief GUI text slider widget
 * @details A widget that displays text with a slider for selecting between multiple positions.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef TEXTSLIDER_H
#define TEXTSLIDER_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"

typedef void (*textslider_slide_cb)(component *c, void *userdata, int pos); ///< Callback when slider position changes

/**
 * @brief Create a text slider widget
 * @param text Label text to display
 * @param help Help text to display
 * @param positions Number of slider positions
 * @param has_off Whether the slider has an "off" position at 0
 * @param cb Callback invoked when position changes
 * @param userdata User data to pass to the callback
 * @return Pointer to the newly created text slider component
 */
component *textslider_create(const char *text, const char *help, unsigned int positions, int has_off,
                             textslider_slide_cb cb, void *userdata);

/**
 * @brief Create a text slider widget with value binding
 * @param text Label text to display
 * @param help Help text to display
 * @param positions Number of slider positions
 * @param has_off Whether the slider has an "off" position at 0
 * @param cb Callback invoked when position changes
 * @param userdata User data to pass to the callback
 * @param bind Pointer to integer that will be updated with the slider position
 * @return Pointer to the newly created text slider component
 */
component *textslider_create_bind(const char *text, const char *help, unsigned int positions, int has_off,
                                  textslider_slide_cb cb, void *userdata, int *bind);

/**
 * @brief Disable audio panning on selection sounds
 * @details By default, the "next" and "previous" item selection sounds are panned.
 *          Call this to disable that effect.
 * @param c Text slider component to modify
 */
void textslider_disable_panning(component *c);

/**
 * @brief Set the font for the text slider
 * @param c Text slider component to modify
 * @param font Font size to use
 */
void textslider_set_font(component *c, font_size font);

/**
 * @brief Set the horizontal text alignment
 * @param c Text slider component to modify
 * @param align Horizontal alignment value
 */
void textslider_set_text_horizontal_align(component *c, text_horizontal_align align);

/**
 * @brief Set the vertical text alignment
 * @param c Text slider component to modify
 * @param align Vertical alignment value
 */
void textslider_set_text_vertical_align(component *c, text_vertical_align align);

#endif // TEXTSLIDER_H
