/**
 * @file progressbar.h
 * @brief GUI progress bar widget
 * @details A progress bar widget with customizable themes for health, endurance, etc.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include "game/gui/component.h"
#include "video/color.h"

/**
 * @brief Progress bar color theme
 */
typedef struct progressbar_theme {
    uint8_t border_topleft_color;     ///< Top-left border color
    uint8_t border_bottomright_color; ///< Bottom-right border color
    uint8_t bg_color;                 ///< Background color
    uint8_t bg_color_alt;             ///< Alternate background color
    uint8_t int_topleft_color;        ///< Interior top-left color
    uint8_t int_bottomright_color;    ///< Interior bottom-right color
    uint8_t int_bg_color;             ///< Interior background color
} progressbar_theme;

extern const progressbar_theme _progressbar_theme_health;    ///< Health bar theme
extern const progressbar_theme _progressbar_theme_endurance; ///< Endurance bar theme
extern const progressbar_theme _progressbar_theme_melee;     ///< Melee bar theme

#define PROGRESSBAR_THEME_HEALTH _progressbar_theme_health       ///< Health bar theme macro
#define PROGRESSBAR_THEME_ENDURANCE _progressbar_theme_endurance ///< Endurance bar theme macro
#define PROGRESSBAR_THEME_MELEE _progressbar_theme_melee         ///< Melee bar theme macro

#define PROGRESSBAR_LEFT 0  ///< Progress bar fills from left
#define PROGRESSBAR_RIGHT 1 ///< Progress bar fills from right

/**
 * @brief Create a progress bar widget
 * @param theme Color theme for the progress bar
 * @param orientation Fill direction (PROGRESSBAR_LEFT or PROGRESSBAR_RIGHT)
 * @param percentage Initial percentage value (0-100)
 * @return Pointer to the newly created progress bar component
 */
component *progressbar_create(progressbar_theme theme, int orientation, int percentage);

/**
 * @brief Set the progress bar value
 * @details The progress bar will animate from current value to new value one percent per tick if animate is true.
 * @param bar Progress bar component to modify
 * @param percentage New percentage value (0-100)
 * @param animate True to animate the transition
 */
void progressbar_set_progress(component *bar, int percentage, bool animate);

/**
 * @brief Set the flashing state
 * @param bar Progress bar component to modify
 * @param flashing Non-zero to enable flashing
 * @param rate Flash rate
 */
void progressbar_set_flashing(component *bar, int flashing, int rate);

/**
 * @brief Set the highlight state
 * @param bar Progress bar component to modify
 * @param highlight True to highlight
 */
void progressbar_set_highlight(component *bar, bool highlight);

#endif // PROGRESSBAR_H
