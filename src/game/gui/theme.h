/*! @file
 * @brief GUI theme configuration for colors and fonts.
 */

#ifndef THEME_H
#define THEME_H

#include "resources/fonts.h"
#include "video/vga_palette.h"

/** @brief Text rendering theme settings. */
typedef struct gui_text_theme {
    font_size font;            ///< Font type/size.
    vga_index primary_color;   ///< Primary text color.
    vga_index secondary_color; ///< Secondary text color.
    vga_index active_color;    ///< Text color when selected.
    vga_index inactive_color;  ///< Text color when not selected.
    vga_index disabled_color;  ///< Text color when disabled.
    vga_index shadow_color;    ///< Text shadow color.
} gui_text_theme;

/** @brief Dialog border theme settings. */
typedef struct gui_dialog_theme {
    vga_index border_color; ///< Primary border color.
} gui_dialog_theme;

/** @brief Combined GUI theme with all settings. */
typedef struct gui_theme {
    gui_dialog_theme dialog;
    gui_text_theme text;
} gui_theme;

/** @brief Initialize a theme with default values. */
inline static void gui_theme_defaults(gui_theme *theme) {
    theme->dialog.border_color = 0;
    theme->text.font = FONT_BIG;
    theme->text.primary_color = 0;
    theme->text.secondary_color = 0;
    theme->text.disabled_color = 0;
    theme->text.active_color = 0;
    theme->text.inactive_color = 0;
    theme->text.shadow_color = 0;
}

#endif // THEME_H
