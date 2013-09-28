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

int game_state_create();
void game_state_free();
int game_state_handle_event(SDL_Event *event);
void game_state_render();
void game_state_tick();
scene* game_state_get_scene();
int game_state_is_running();
void game_state_set_next(unsigned int next_scene_id);
void game_state_add_object(object *obj);
game_player* game_state_get_player(int player_id);
unsigned int game_state_ms_per_tick();

#endif // _GAME_STATE_H
