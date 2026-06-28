#ifndef INTERSECT_H
#define INTERSECT_H

#include "game/protos/object.h"

int intersect_object_object(const object *a, const object *b);
int intersect_object_point(const object *obj, vec2i point);
int intersect_sprite_hitpoint(const object *obj, const object *target, int level, vec2i *point);
int intersect_har_sprite_hitpoint(const object *obj, const object *target, int level, vec2i *point);

#endif // INTERSECT_H
