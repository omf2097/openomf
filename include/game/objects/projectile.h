#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "game/protos/object.h"

typedef struct projectile_local_t {
	int _empty;
} projectile_local;

int projectile_create(object *obj);

#endif // _PROJECTILE_H