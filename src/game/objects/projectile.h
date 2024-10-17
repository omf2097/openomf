#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "game/objects/har.h"
#include "game/protos/object.h"

typedef struct har_t har;

int projectile_create(object *obj, har *har);
const af *projectile_get_af_data(object *obj);
uint8_t projectile_get_owner(object *obj);
void projectile_set_wall_bounce(object *obj, int bounce);
void projectile_set_invincible(object *obj);
void projectile_stop_on_ground(object *obj, int stop);

void projectile_mark_hit(object *obj);
bool projectile_did_hit(object *obj);
void projectile_clear_hit(object *obj);

void projectile_link_object(object *obj, object *link);

#endif // PROJECTILE_H
