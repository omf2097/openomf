#ifndef _ARENA_H
#define _ARENA_H

#include "game/protos/scene.h"

enum {
    ARENA_STATE_STARTING,
    ARENA_STATE_FIGHTING,
    ARENA_STATE_ENDING
};

int arena_create(scene *scene);
int arena_get_state(scene *scene);
void arena_set_state(scene *scene, int state);
palette* arena_get_player_palette(scene *scene, int player);

#endif // _ARENA_H
