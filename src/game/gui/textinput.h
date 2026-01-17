/**
 * @file textinput.h
 * @brief GUI text input widget
 * @details A widget for accepting user text input with customizable filtering and callbacks.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"

typedef void (*textinput_done_cb)(component *c, void *userdata); ///< Callback when input is completed
typedef bool (*textinput_filter_cb)(char c);                     ///< Callback to filter input characters

/**
 * @brief Create a text input widget
 * @param max_chars Maximum number of characters allowed
 * @param help Help text to display
 * @param initial_value Initial text value
 * @return Pointer to the newly created text input component
 */
component *textinput_create(int max_chars, const char *help, const char *initial_value);

/**
 * @brief Get the current text value
 * @param c Text input component to query
 * @return Current text value
 */
const char *textinput_value(const component *c);

/**
 * @brief Clear the text input
 * @param c Text input component to clear
 */
void textinput_clear(component *c);

/**
 * @brief Enable or disable background rendering
 * @param c Text input component to modify
 * @param enabled Non-zero to enable background, zero to disable
 */
void textinput_enable_background(component *c, int enabled);

/**
 * @brief Set a filter callback for input characters
 * @param c Text input component to modify
 * @param filter_cb Filter callback that returns true to accept a character
 */
void textinput_set_filter_cb(component *c, textinput_filter_cb filter_cb);

/**
 * @brief Set the done callback
 * @param c Text input component to modify
 * @param done_cb Callback invoked when input is completed (Enter pressed)
 * @param userdata User data to pass to the callback
 */
void textinput_set_done_cb(component *c, textinput_done_cb done_cb, void *userdata);

/**
 * @brief Set the text content
 * @param c Text input component to modify
 * @param value Text value to set
 */
void textinput_set_text(component *c, char const *value);

/**
 * @brief Set the font for the text input
 * @param c Text input component to modify
 * @param font Font size to use
 */
void textinput_set_font(component *c, font_size font);

/**
 * @brief Set the horizontal text alignment
 * @param c Text input component to modify
 * @param align Horizontal alignment value
 */
void textinput_set_horizontal_align(component *c, text_horizontal_align align);

/**
 * @brief Set the text shadow style and color
 * @param c Text input component to modify
 * @param shadow Shadow style bitmask
 * @param color VGA palette color index for shadow
 */
void textinput_set_text_shadow(component *c, uint8_t shadow, vga_index color);

#endif // TEXTINPUT_H
