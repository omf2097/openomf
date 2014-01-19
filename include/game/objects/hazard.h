#ifndef _HAZARD_H
#define _HAZARD_H

#include "game/protos/object.h"
#include "game/game_state_type.h"
#include "resources/bk.h"
#include "resources/bk_info.h"

typedef struct har_t har;

int hazard_create(object *obj, scene *scene);
void hazard_bootstrap(object *obj);

#endif // _HAZARD_H
