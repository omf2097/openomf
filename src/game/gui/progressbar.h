/*! @file
 * @brief Animated progress bar with smooth transitions and flashing effects.
 */

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "game/gui/component.h"
#include "video/color.h"

/** @brief Color theme for progress bar. */
typedef struct progressbar_theme {
    uint8_t border_topleft_color;     ///< Border color for top and left edges.
    uint8_t border_bottomright_color; ///< Border color for bottom and right edges.
    uint8_t bg_color;                 ///< Primary background color.
    uint8_t bg_color_alt;             ///< Alternate background color (for patterns).
    uint8_t int_topleft_color;        ///< Interior bar top-left highlight color.
    uint8_t int_bottomright_color;    ///< Interior bar bottom-right shadow color.
    uint8_t int_bg_color;             ///< Interior bar fill color.
} progressbar_theme;

/** @brief Health bar theme (red/orange colors). */
extern const progressbar_theme _progressbar_theme_health;

/** @brief Endurance bar theme (blue colors). */
extern const progressbar_theme _progressbar_theme_endurance;

/** @brief Melee bar theme (yellow colors). */
extern const progressbar_theme _progressbar_theme_melee;

/** @brief Health bar theme constant. */
#define PROGRESSBAR_THEME_HEALTH _progressbar_theme_health

/** @brief Endurance bar theme constant. */
#define PROGRESSBAR_THEME_ENDURANCE _progressbar_theme_endurance

/** @brief Melee bar theme constant. */
#define PROGRESSBAR_THEME_MELEE _progressbar_theme_melee

/** @brief Left-to-right fill orientation. */
#define PROGRESSBAR_LEFT 0

/** @brief Right-to-left fill orientation. */
#define PROGRESSBAR_RIGHT 1

/** @brief Create a progress bar widget. */
component *progressbar_create(progressbar_theme theme, int orientation, int percentage);

/** @brief Set the progress percentage. Animates if animate is true. */
void progressbar_set_progress(component *bar, int percentage, bool animate);

/** @brief Enable or disable flashing effect. */
void progressbar_set_flashing(component *bar, int flashing, int rate);

/** @brief Enable or disable highlight mode. */
void progressbar_set_highlight(component *bar, bool highlight);

#endif // PROGRESSBAR_H
