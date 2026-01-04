/*! @file
 * @brief Button widget displaying a sprite image with optional text overlay.
 */

#ifndef SPRITEBUTTON_H
#define SPRITEBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"
#include "video/surface.h"

/** @brief Click callback type. */
typedef void (*spritebutton_click_cb)(component *c, void *userdata);

/** @brief Per-frame tick callback type. */
typedef void (*spritebutton_tick_cb)(component *c, void *userdata);

/** @brief Focus change callback type. */
typedef void (*spritebutton_focus_cb)(component *c, bool focused, void *userdata);

/** @brief Create a sprite button widget. */
component *spritebutton_create(const char *text, const surface *img, bool disabled, spritebutton_click_cb cb,
                               void *userdata);

/** @brief Set horizontal text alignment. */
void spritebutton_set_horizontal_align(component *c, text_horizontal_align align);

/** @brief Set vertical text alignment. */
void spritebutton_set_vertical_align(component *c, text_vertical_align align);

/** @brief Set text direction. */
void spritebutton_set_text_direction(component *c, text_row_direction direction);

/** @brief Set text margins. */
void spritebutton_set_text_margin(component *c, text_margin margins);

/** @brief Set text color (overrides theme). */
void spritebutton_set_text_color(component *c, vga_index color);

/** @brief Set text font. */
void spritebutton_set_font(component *c, font_size font);

/** @brief Get the current sprite surface. */
const surface *spritebutton_get_img(const component *c);

/** @brief Set a new sprite surface. */
void spritebutton_set_img(component *c, const surface *img);

/** @brief Set per-frame tick callback. */
void spritebutton_set_tick_cb(component *c, spritebutton_tick_cb);

/** @brief Set focus change callback. */
void spritebutton_set_focus_cb(component *c, spritebutton_focus_cb);

/** @brief Make sprite always visible (never hide after activation). */
void spritebutton_set_always_display(component *c);

/** @brief Set whether to free userdata when button is freed. */
void spritebutton_set_free_userdata(component *c, bool free_userdata);

#endif // SPRITEBUTTON_H
