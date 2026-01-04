/*! @file
 * @brief Menu background surface creation utilities.
 */

#ifndef MENU_BACKGROUND_H
#define MENU_BACKGROUND_H

#include "video/surface.h"

/**
 * @brief Menu background visual styles.
 */
typedef enum menu_background_style
{
    MenuBackground,         /**< Blue borders, coarse grid pattern */
    MenuBackgroundMeleeVs,  /**< Green borders, finer grid pattern */
    MenuBackgroundNewsroom, /**< Blue borders, no grid pattern */
} menu_background_style;

/**
 * @brief Create a transparent background surface.
 * @param s Surface to initialize.
 * @param w Width in pixels.
 * @param h Height in pixels.
 */
void menu_transparent_bg_create(surface *s, int w, int h);

/**
 * @brief Create a styled menu background surface.
 * @param sur Surface to initialize.
 * @param w Width in pixels.
 * @param h Height in pixels.
 * @param style Background style to use.
 */
void menu_background_create(surface *sur, int w, int h, menu_background_style style);

/**
 * @brief Create a simple bordered background surface.
 * @param sur Surface to initialize.
 * @param w Width in pixels.
 * @param h Height in pixels.
 * @param color Border color (VGA palette index).
 */
void menu_background_border_create(surface *sur, int w, int h, vga_index color);

#endif // MENU_BACKGROUND_H
