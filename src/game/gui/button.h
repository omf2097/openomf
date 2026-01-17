/**
 * @file button.h
 * @brief GUI text button widget
 * @details A simple text-based button widget with click callback support.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef BUTTON_H
#define BUTTON_H

#include "game/gui/component.h"

typedef void (*button_click_cb)(component *c, void *userdata); ///< Button click callback

/**
 * @brief Create a button widget
 * @param text Text to display on the button
 * @param help Help text to display when focused
 * @param disabled Whether the button starts disabled
 * @param border Whether to draw a border around the button
 * @param cb Click callback function
 * @param userdata User data passed to callbacks
 * @return Pointer to the newly created button component
 */
component *button_create(const char *text, const char *help, bool disabled, bool border, button_click_cb cb,
                         void *userdata);

/**
 * @brief Set the button text
 * @param c Button component to modify
 * @param text New text to display
 */
void button_set_text(component *c, const char *text);

/**
 * @brief Set the button's user data
 * @param c Button component to modify
 * @param userdata New user data pointer
 */
void button_set_userdata(component *c, void *userdata);

/**
 * @brief Set the text shadow style and color
 * @param c Button component to modify
 * @param shadow Shadow style bitmask
 * @param color VGA palette color index for shadow
 */
void button_set_text_shadow(component *c, uint8_t shadow, vga_index color);

#endif // BUTTON_H
