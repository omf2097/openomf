#ifndef HAZARD_H
#define HAZARD_H

#include "game/game_state_type.h"
#include "game/protos/object.h"
#include "resources/bk.h"
#include "resources/bk_info.h"

typedef struct har_t har;

int hazard_create(object *obj, scene *scene);

#endif // HAZARD_H
