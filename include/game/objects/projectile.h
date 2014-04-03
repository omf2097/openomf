#ifndef _PROJECTILE_H
#define _PROJECTILE_H

#include "game/protos/object.h"
#include "game/objects/har.h"

typedef struct har_t har;

int projectile_create(object *obj);
af *projectile_get_af_data(object *obj);
object *projectile_get_owner(object *obj);
void projectile_bootstrap(object *obj);

#endif // _PROJECTILE_H
