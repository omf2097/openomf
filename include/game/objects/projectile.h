#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "game/protos/object.h"

typedef struct har_t har;

int projectile_create(object *obj);
har *projectile_get_har(object *obj);

#endif // _PROJECTILE_H
