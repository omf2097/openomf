#ifndef SPRITEIMAGE_H
#define SPRITEIMAGE_H

#include "game/gui/component.h"
#include "video/surface.h"

component *spriteimage_create(surface *img);
void spriteimage_set_owns_sprite(component *c, bool owns_sprite);

#endif // SPRITEIMAGE_H
