/**
 * @file label.h
 * @brief GUI label widget
 * @details A widget for displaying static or dynamic text.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef LABEL_H
#define LABEL_H

#include "game/gui/component.h"
#include "game/gui/text/text.h"

/**
 * @brief Create a label with a maximum width constraint
 * @param text Text content to display
 * @param max_width Maximum width in pixels for word wrapping
 * @return Pointer to the newly created label component
 */
component *label_create_with_width(const char *text, uint16_t max_width);

/**
 * @brief Create a title-style label
 * @param text Text content to display
 * @return Pointer to the newly created title label component
 */
component *label_create_title(const char *text);

/**
 * @brief Create a standard label
 * @param text Text content to display
 * @return Pointer to the newly created label component
 */
component *label_create(const char *text);

/**
 * @brief Set the text content of a label
 * @param label Label component to modify
 * @param text New text content
 */
void label_set_text(component *label, const char *text);

/**
 * @brief Set the text color of a label
 * @param label Label component to modify
 * @param color VGA palette color index
 */
void label_set_text_color(component *label, vga_index color);

/**
 * @brief Set the horizontal text alignment
 * @param c Label component to modify
 * @param align Horizontal alignment value
 */
void label_set_text_horizontal_align(component *c, text_horizontal_align align);

/**
 * @brief Set the vertical text alignment
 * @param c Label component to modify
 * @param align Vertical alignment value
 */
void label_set_text_vertical_align(component *c, text_vertical_align align);

/**
 * @brief Set the letter spacing
 * @param c Label component to modify
 * @param spacing Spacing in pixels between letters
 */
void label_set_text_letter_spacing(component *c, uint8_t spacing);

/**
 * @brief Set the text shadow style and color
 * @param c Label component to modify
 * @param shadow Shadow style bitmask
 * @param color VGA palette color index for shadow
 */
void label_set_text_shadow(component *c, uint8_t shadow, vga_index color);

/**
 * @brief Set the font for the label
 * @param label Label component to modify
 * @param font Font size to use
 */
void label_set_font(component *label, font_size font);

/**
 * @brief Set the margin around the text
 * @param c Label component to modify
 * @param margin Margin values
 */
void label_set_margin(component *c, text_margin margin);

/**
 * @brief Set the color theme for the label
 * @param c Label component to modify
 * @param theme Theme index (0 = primary, 1 = secondary)
 */
void label_set_color_theme(component *c, int theme);

#endif // LABEL_H
