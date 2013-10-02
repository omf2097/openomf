#ifndef _INTERSECT_H
#define _INTERSECT_H

#include "game/protos/object.h"

int intersect_object_object(object *a, object *b);
int intersect_object_point(object *obj, vec2i point);
int intersect_sprite_hitpoint(object *obj, object *target, int color_range_start, int color_range_end);

#endif // _INTERSECT_H