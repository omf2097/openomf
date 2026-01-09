/*! @file
 * @brief Simple widget that displays a static sprite image.
 */

#ifndef SPRITEIMAGE_H
#define SPRITEIMAGE_H

#include "game/gui/component.h"
#include "video/surface.h"

/** @brief Create a sprite image widget. */
component *spriteimage_create(const surface *img);

/** @brief Set whether the widget owns the sprite surface. */
void spriteimage_set_owns_sprite(component *c, bool owns_sprite);

#endif // SPRITEIMAGE_H
