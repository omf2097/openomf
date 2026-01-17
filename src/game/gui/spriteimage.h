/**
 * @file spriteimage.h
 * @brief GUI sprite image widget
 * @details A simple widget for displaying a static sprite image.
 * @copyright MIT License
 * @date 2026
 * @author OpenOMF Project
 */

#ifndef SPRITEIMAGE_H
#define SPRITEIMAGE_H

#include "game/gui/component.h"
#include "video/surface.h"

/**
 * @brief Create a sprite image widget
 * @param img Sprite surface to display
 * @return Pointer to the newly created sprite image component
 */
component *spriteimage_create(const surface *img);

/**
 * @brief Set whether the widget owns the sprite memory
 * @param c Sprite image component to modify
 * @param owns_sprite True if the widget should free the sprite on destruction
 */
void spriteimage_set_owns_sprite(component *c, bool owns_sprite);

#endif // SPRITEIMAGE_H
