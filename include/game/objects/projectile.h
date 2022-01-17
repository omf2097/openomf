#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "game/objects/har.h"
#include "game/protos/object.h"

typedef struct har_t har;

int projectile_create(object *obj);
af *projectile_get_af_data(object *obj);
object *projectile_get_owner(object *obj);
void projectile_bootstrap(object *obj);
void projectile_set_wall_bounce(object *obj, int bounce);
void projectile_set_invincible(object *obj);
void projectile_stop_on_ground(object *obj, int stop);

#endif // PROJECTILE_H
