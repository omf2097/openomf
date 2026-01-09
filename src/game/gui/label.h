/*! @file
 * @brief Non-interactive text label widget.
 */

#ifndef LABEL_H
#define LABEL_H

#include "game/gui/component.h"
#include "game/gui/text/text.h"

/** @brief Create a label with word wrapping at the specified width. */
component *label_create_with_width(const char *text, uint16_t max_width);

/** @brief Create a centered title label using secondary color. */
component *label_create_title(const char *text);

/** @brief Create a simple label. */
component *label_create(const char *text);

/** @brief Change the label text. */
void label_set_text(component *label, const char *text);

/** @brief Set a custom text color (overrides theme). */
void label_set_text_color(component *label, vga_index color);

/** @brief Set horizontal text alignment. */
void label_set_text_horizontal_align(component *c, text_horizontal_align align);

/** @brief Set vertical text alignment. */
void label_set_text_vertical_align(component *c, text_vertical_align align);

/** @brief Set letter spacing. */
void label_set_text_letter_spacing(component *c, uint8_t spacing);

/** @brief Configure text shadow effect. */
void label_set_text_shadow(component *c, uint8_t shadow, vga_index color);

/** @brief Set the font (overrides theme). */
void label_set_font(component *label, font_size font);

/** @brief Set text margins. */
void label_set_margin(component *c, text_margin margin);

/** @brief Set which theme color to use (0 = primary, 1 = secondary). */
void label_set_color_theme(component *c, int theme);

#endif // LABEL_H
