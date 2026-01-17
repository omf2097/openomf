/**
 * @file menu_background.h
 * @brief Menu background rendering
 * @details Functions for creating styled menu backgrounds and borders.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef MENU_BACKGROUND_H
#define MENU_BACKGROUND_H

#include "video/surface.h"

/**
 * @brief Menu background style options
 */
typedef enum menu_background_style
{
    MenuBackground,         ///< Blue borders, coarse grid
    MenuBackgroundMeleeVs,  ///< Green borders, finer grid
    MenuBackgroundNewsroom, ///< Blue borders, no grid
} menu_background_style;

/**
 * @brief Create a transparent menu background
 * @param s Surface to draw the background onto
 * @param w Width of the background
 * @param h Height of the background
 */
void menu_transparent_bg_create(surface *s, int w, int h);

/**
 * @brief Create a styled menu background
 * @param sur Surface to draw the background onto
 * @param w Width of the background
 * @param h Height of the background
 * @param style Background style to use
 */
void menu_background_create(surface *sur, int w, int h, menu_background_style);

/**
 * @brief Create a menu border
 * @param sur Surface to draw the border onto
 * @param w Width of the bordered area
 * @param h Height of the bordered area
 * @param color VGA palette color index for the border
 */
void menu_background_border_create(surface *sur, int w, int h, vga_index color);

#endif // MENU_BACKGROUND_H
