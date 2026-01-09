/*! @file
 * @brief Text entry widget with filtering and completion callbacks.
 */

#ifndef TEXTINPUT_H
#define TEXTINPUT_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"

/** @brief Callback invoked when text entry is complete. */
typedef void (*textinput_done_cb)(component *c, void *userdata);

/** @brief Callback to filter individual characters during input. */
typedef bool (*textinput_filter_cb)(char c);

/** @brief Create a text input widget. */
component *textinput_create(int max_chars, const char *help, const char *initial_value);

/** @brief Get the current text value. */
const char *textinput_value(const component *c);

/** @brief Clear the text content. */
void textinput_clear(component *c);

/** @brief Enable or disable background rendering. */
void textinput_enable_background(component *c, int enabled);

/** @brief Set the character filter callback. */
void textinput_set_filter_cb(component *c, textinput_filter_cb filter_cb);

/** @brief Set the completion callback. */
void textinput_set_done_cb(component *c, textinput_done_cb done_cb, void *userdata);

/** @brief Set the text content. */
void textinput_set_text(component *c, char const *value);

/** @brief Set the text font. */
void textinput_set_font(component *c, font_size font);

/** @brief Set horizontal text alignment. */
void textinput_set_horizontal_align(component *c, text_horizontal_align align);

/** @brief Configure text shadow effect. */
void textinput_set_text_shadow(component *c, uint8_t shadow, vga_index color);

#endif // TEXTINPUT_H
