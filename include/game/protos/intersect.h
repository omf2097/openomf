#ifndef _INTERSECT_H
#define _INTERSECT_H

#include "game/protos/object.h"

int intersect_object_object(object *a, object *b);
int intersect_object_point(object *obj, vec2i point);
int intersect_sprite_point(object *obj, vec2i point, int range_color_start, int range_color_end);

#endif // _INTERSECT_H