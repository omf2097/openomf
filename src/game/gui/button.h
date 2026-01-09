/*! @file
 * @brief Clickable button widget with text label.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "game/gui/component.h"

/** @brief Callback invoked when button is activated. */
typedef void (*button_click_cb)(component *c, void *userdata);

/** @brief Create a button widget. */
component *button_create(const char *text, const char *help, bool disabled, bool border, button_click_cb cb,
                         void *userdata);

/** @brief Change the button label text. */
void button_set_text(component *c, const char *text);

/** @brief Set the user data passed to the click callback. */
void button_set_userdata(component *c, void *userdata);

/** @brief Configure text shadow effect. */
void button_set_text_shadow(component *c, uint8_t shadow, vga_index color);

#endif // BUTTON_H
