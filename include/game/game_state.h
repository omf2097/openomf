#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include <SDL2/SDL.h>
#include "utils/vector.h"
#include "game/objects/har.h"
#include "game/protos/scene.h"
#include "game/protos/object.h"
#include "controller/controller.h"

typedef struct game_player_t {
    int har_id;
    int player_id;
    har *har;
    controller *ctrl;
    texture *portrait;
    int selectable;
    // store crap like agility and stuff here?
} game_player;

typedef struct game_state_t {
    unsigned int run;
    unsigned int this_id, next_id;
    scene sc;
    vector objects;
    scene_player players[2];
} game_state;


int game_state_create(game_state *game);
void game_state_free(game_state *game);
int game_state_handle_event(game_state *game, SDL_Event *event);
void game_state_render(game_state *game);
void game_state_tick(game_state *game);
int game_state_is_running(game_state *game);
void game_state_set_player_har(game_state *game, int player_id, har *har);
void game_state_set_player_ctrl(game_state *game, int player_id, controller *ctrl);
unsigned int game_state_ms_per_tick(game_state *game);

#endif // _GAME_STATE_H
