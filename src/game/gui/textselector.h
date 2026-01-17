/**
 * @file textselector.h
 * @brief GUI text selector widget
 * @details A widget for selecting between text options with left/right navigation.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef TEXTSELECTOR_H
#define TEXTSELECTOR_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"
#include "utils/vector.h"

typedef void (*textselector_toggle_cb)(component *c, void *userdata, int pos); ///< Callback when selection changes

/**
 * @brief Create a text selector widget
 * @param text Label text to display
 * @param help Help text to display when focused
 * @param toggle_cb Callback invoked when selection changes
 * @param userdata User data for callbacks
 * @return Pointer to the newly created text selector component
 */
component *textselector_create(const char *text, const char *help, textselector_toggle_cb toggle_cb, void *userdata);

/**
 * @brief Create a text selector widget with value binding
 * @param text Label text to display
 * @param help Help text to display when focused
 * @param toggle_cb Callback invoked when selection changes
 * @param userdata User data for callbacks
 * @param bind Pointer to integer that will be updated with the selected index
 * @return Pointer to the newly created text selector component
 */
component *textselector_create_bind(const char *text, const char *help, textselector_toggle_cb toggle_cb,
                                    void *userdata, int *bind);

/**
 * @brief Create a text selector widget with value binding and predefined options
 * @param text Label text to display
 * @param help Help text to display when focused
 * @param toggle_cb Callback invoked when selection changes
 * @param userdata User data for callbacks
 * @param bind Pointer to integer that will be updated with the selected index
 * @param opts Array of option strings
 * @param opt_size Number of options
 * @return Pointer to the newly created text selector component
 */
component *textselector_create_bind_opts(const char *text, const char *help, textselector_toggle_cb toggle_cb,
                                         void *userdata, int *bind, const char **opts, int opt_size);

/**
 * @brief Add an option to the selector
 * @param c Text selector component to modify
 * @param option Option text to add
 */
void textselector_add_option(component *c, const char *option);

/**
 * @brief Clear all options from the selector
 * @param c Text selector component to modify
 */
void textselector_clear_options(component *c);

/**
 * @brief Get the currently selected option text
 * @param c Text selector component to query
 * @return Currently selected option string
 */
const char *textselector_get_current_text(const component *c);

/**
 * @brief Get the current selection index
 * @param c Text selector component to query
 * @return Current selection index
 */
int textselector_get_pos(const component *c);

/**
 * @brief Set the current selection index
 * @param c Text selector component to modify
 * @param pos New selection index
 */
void textselector_set_pos(component *c, int pos);

/**
 * @brief Set the font for the text selector
 * @param c Text selector component to modify
 * @param font Font size to use
 */
void textselector_set_font(component *c, font_size font);

/**
 * @brief Set the horizontal text alignment
 * @param c Text selector component to modify
 * @param align Horizontal alignment value
 */
void textselector_set_text_horizontal_align(component *c, text_horizontal_align align);

/**
 * @brief Set the vertical text alignment
 * @param c Text selector component to modify
 * @param align Vertical alignment value
 */
void textselector_set_text_vertical_align(component *c, text_vertical_align align);

#endif // TEXTSELECTOR_H
