/**
 * @file spritebutton.h
 * @brief GUI sprite button widget
 * @details A button widget that displays a sprite image with optional text overlay.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SPRITEBUTTON_H
#define SPRITEBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"
#include "video/surface.h"

typedef void (*spritebutton_click_cb)(component *c, void *userdata);               ///< Button click callback
typedef void (*spritebutton_tick_cb)(component *c, void *userdata);                ///< Button tick callback
typedef void (*spritebutton_focus_cb)(component *c, bool focused, void *userdata); ///< Button focus change callback

/**
 * @brief Create a sprite button widget
 * @param text Text to display on the button
 * @param img Sprite image for the button
 * @param disabled Whether the button starts disabled
 * @param cb Click callback
 * @param userdata User data for callbacks
 * @return Pointer to the newly created sprite button component
 */
component *spritebutton_create(const char *text, const surface *img, bool disabled, spritebutton_click_cb cb,
                               void *userdata);

/**
 * @brief Set the horizontal text alignment
 * @param c Sprite button component to modify
 * @param align Horizontal alignment value
 */
void spritebutton_set_horizontal_align(component *c, text_horizontal_align align);

/**
 * @brief Set the vertical text alignment
 * @param c Sprite button component to modify
 * @param align Vertical alignment value
 */
void spritebutton_set_vertical_align(component *c, text_vertical_align align);

/**
 * @brief Set the text direction
 * @param c Sprite button component to modify
 * @param direction Text direction (horizontal or vertical)
 */
void spritebutton_set_text_direction(component *c, text_row_direction direction);

/**
 * @brief Set the text margin
 * @param c Sprite button component to modify
 * @param margins Margin values
 */
void spritebutton_set_text_margin(component *c, text_margin margins);

/**
 * @brief Set the text color
 * @param c Sprite button component to modify
 * @param color VGA palette color index
 */
void spritebutton_set_text_color(component *c, vga_index color);

/**
 * @brief Set the font
 * @param c Sprite button component to modify
 * @param font Font size to use
 */
void spritebutton_set_font(component *c, font_size font);

/**
 * @brief Get the button's sprite image
 * @param c Sprite button component to query
 * @return Pointer to the sprite surface
 */
const surface *spritebutton_get_img(const component *c);

/**
 * @brief Set the button's sprite image
 * @param c Sprite button component to modify
 * @param img New sprite image
 */
void spritebutton_set_img(component *c, const surface *img);

/**
 * @brief Set the tick callback
 * @param c Sprite button component to modify
 * @param Tick callback function
 */
void spritebutton_set_tick_cb(component *c, spritebutton_tick_cb);

/**
 * @brief Set the focus callback
 * @param c Sprite button component to modify
 * @param Focus callback function
 */
void spritebutton_set_focus_cb(component *c, spritebutton_focus_cb);

/**
 * @brief Set the button to always display (even when not focused)
 * @param c Sprite button component to modify
 */
void spritebutton_set_always_display(component *c);

/**
 * @brief Set whether to free userdata when the button is freed
 * @param c Sprite button component to modify
 * @param free_userdata True to free userdata on destruction
 */
void spritebutton_set_free_userdata(component *c, bool free_userdata);

#endif // SPRITEBUTTON_H
