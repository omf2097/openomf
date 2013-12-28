#ifndef _GAME_STATE_H
#define _GAME_STATE_H

#include <SDL2/SDL.h>
#include "utils/vector.h"
#include "utils/random.h"
#include "game/serial.h"
#include "game/game_state_type.h"

typedef struct scene_t scene;
typedef struct game_player_t game_player;
typedef struct object_t object;

enum {
    RENDER_LAYER_BOTTOM = 0,
    RENDER_LAYER_MIDDLE,
    RENDER_LAYER_TOP
};

enum {
    ROLE_CLIENT,
    ROLE_SERVER
};

enum {
    NET_MODE_NONE,
    NET_MODE_CLIENT,
    NET_MODE_SERVER
};

int game_state_create(game_state *gs, int net_mode);
void game_state_free(game_state *gs);
int game_state_handle_event(game_state *gs, SDL_Event *event);
void game_state_render(game_state *gs);
void game_state_tick(game_state *gs);
unsigned int game_state_get_tick(game_state *gs);
scene* game_state_get_scene(game_state *gs);
int game_state_is_running(game_state *gs);
void game_state_set_next(game_state *gs, unsigned int next_scene_id);
game_player* game_state_get_player(game_state *gs, int player_id);
int game_state_num_players(game_state *gs);
int game_state_ms_per_tick(game_state *gs);
int game_state_serialize(game_state *gs, serial *ser);
int game_state_unserialize(game_state *gs, serial *ser, int rtt);

void game_state_add_object(game_state *gs, object *obj, int layer);
void game_state_del_object(game_state *gs, object *obj);
void game_state_del_animation(game_state *gs, int anim_id);

#endif // _GAME_STATE_H
