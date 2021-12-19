#ifndef INTERSECT_H
#define INTERSECT_H

#include "game/protos/object.h"

#ifdef DEBUGMODE
#include "video/image.h"
#endif

int intersect_object_object(object *a, object *b);
int intersect_object_point(object *obj, vec2i point);
int intersect_sprite_hitpoint(object *obj, object *target, int level, vec2i *point);


#endif // INTERSECT_H
