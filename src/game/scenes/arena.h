#ifndef ARENA_H
#define ARENA_H

#include "game/protos/scene.h"

enum
{
    ARENA_STATE_STARTING,
    ARENA_STATE_FIGHTING,
    ARENA_STATE_ENDING
};

int arena_create(scene *scene);
int arena_get_state(scene *scene);
void arena_set_state(scene *scene, int state);
vga_palette *arena_get_player_palette(scene *scene, int player);
void arena_toggle_rein(scene *scene);
void maybe_install_har_hooks(scene *scene);
uint32_t arena_state_hash(game_state *gs);
void arena_state_dump(game_state *gs, char *buf);
void arena_reset(scene *sc);

#endif // ARENA_H
