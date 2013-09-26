#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include <SDL2/SDL.h>
#include "utils/vector.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "game/game_player.h"

typedef struct game_state_t {
    unsigned int run;
    unsigned int this_id, next_id;
    scene sc;
    vector objects;
    game_player players[2];
} game_state;

int game_state_create(game_state *game);
void game_state_free(game_state *game);
int game_state_handle_event(game_state *game, SDL_Event *event);
void game_state_render(game_state *game);
void game_state_tick(game_state *game);
int game_state_is_running(game_state *game);
void game_state_set_next(game_state *game, unsigned int next_scene_ud);
void game_state_add_object(game_state *game, object *obj);
game_player* game_state_get_player(game_state *game, int player_id);
unsigned int game_state_ms_per_tick(game_state *game);

#endif // _GAME_STATE_H
