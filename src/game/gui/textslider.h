/*! @file
 * @brief Visual slider widget for numeric value selection.
 */

#ifndef TEXTSLIDER_H
#define TEXTSLIDER_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"

/** @brief Callback invoked when slider position changes. */
typedef void (*textslider_slide_cb)(component *c, void *userdata, int pos);

/** @brief Create a text slider widget. */
component *textslider_create(const char *text, const char *help, unsigned int positions, int has_off,
                             textslider_slide_cb cb, void *userdata);

/** @brief Create a text slider bound to an integer variable. */
component *textslider_create_bind(const char *text, const char *help, unsigned int positions, int has_off,
                                  textslider_slide_cb cb, void *userdata, int *bind);

/** @brief Disable audio panning when changing positions. */
void textslider_disable_panning(component *c);

/** @brief Set the text font. */
void textslider_set_font(component *c, font_size font);

/** @brief Set horizontal text alignment. */
void textslider_set_text_horizontal_align(component *c, text_horizontal_align align);

/** @brief Set vertical text alignment. */
void textslider_set_text_vertical_align(component *c, text_vertical_align align);

#endif // TEXTSLIDER_H
