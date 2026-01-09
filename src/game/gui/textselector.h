/*! @file
 * @brief Dropdown-style selector widget with text options.
 */

#ifndef TEXTSELECTOR_H
#define TEXTSELECTOR_H

#include "game/gui/component.h"
#include "game/gui/text/enums.h"
#include "utils/vector.h"

/** @brief Callback invoked when selection changes. */
typedef void (*textselector_toggle_cb)(component *c, void *userdata, int pos);

/** @brief Create a text selector widget. */
component *textselector_create(const char *text, const char *help, textselector_toggle_cb toggle_cb, void *userdata);

/** @brief Create a text selector bound to an integer variable. */
component *textselector_create_bind(const char *text, const char *help, textselector_toggle_cb toggle_cb,
                                    void *userdata, int *bind);

/** @brief Create a text selector with bound variable and initial options. */
component *textselector_create_bind_opts(const char *text, const char *help, textselector_toggle_cb toggle_cb,
                                         void *userdata, int *bind, const char **opts, int opt_size);

/** @brief Add an option to the selector. */
void textselector_add_option(component *c, const char *option);

/** @brief Remove all options from the selector. */
void textselector_clear_options(component *c);

/** @brief Get the text of the currently selected option. */
const char *textselector_get_current_text(const component *c);

/** @brief Get the index of the currently selected option. */
int textselector_get_pos(const component *c);

/** @brief Set the selected option by index. */
void textselector_set_pos(component *c, int pos);

/** @brief Set the text font. */
void textselector_set_font(component *c, font_size font);

/** @brief Set horizontal text alignment. */
void textselector_set_text_horizontal_align(component *c, text_horizontal_align align);

/** @brief Set vertical text alignment. */
void textselector_set_text_vertical_align(component *c, text_vertical_align align);

#endif // TEXTSELECTOR_H
