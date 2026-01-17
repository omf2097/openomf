/**
 * @file theme.h
 * @brief GUI theming support
 * @details Structures and functions for theming GUI components including colors and fonts.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef THEME_H
#define THEME_H

#include "resources/fonts.h"
#include "video/vga_palette.h"

/**
 * @brief Text theme settings
 */
typedef struct gui_text_theme {
    font_size font;            ///< Font type/size
    vga_index primary_color;   ///< Primary text color
    vga_index secondary_color; ///< Secondary text color
    vga_index active_color;    ///< Text color when selected (e.g. buttons)
    vga_index inactive_color;  ///< Text color when NOT selected (e.g. buttons)
    vga_index disabled_color;  ///< Text color when disabled (e.g. buttons)
    vga_index shadow_color;    ///< Text shadow color (if enabled)
} gui_text_theme;

/**
 * @brief Dialog theme settings
 */
typedef struct gui_dialog_theme {
    vga_index border_color; ///< Primary border color (e.g. dialog borders)
} gui_dialog_theme;

/**
 * @brief Complete GUI theme
 */
typedef struct gui_theme {
    gui_dialog_theme dialog; ///< Dialog-specific theme settings
    gui_text_theme text;     ///< Text-specific theme settings
} gui_theme;

/**
 * @brief Initialize a theme with default values
 * @param theme Theme structure to initialize
 */
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
