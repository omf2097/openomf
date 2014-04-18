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

int game_state_create(game_state *gs, int net_mode);
void game_state_free(game_state *gs);
int game_state_handle_event(game_state *gs, SDL_Event *event);
void game_state_render(game_state *gs);
void game_state_static_tick(game_state *gs);
void game_state_dynamic_tick(game_state *gs);
void game_state_tick_controllers(game_state *gs);
unsigned int game_state_get_tick(game_state *gs);
scene* game_state_get_scene(game_state *gs);
int game_state_is_running(game_state *gs);
void game_state_set_next(game_state *gs, unsigned int next_scene_id);
game_player* game_state_get_player(game_state *gs, int player_id);
int game_state_num_players(game_state *gs);
void game_state_init_demo(game_state *gs);
int game_state_ms_per_dyntick(game_state *gs);
ticktimer* game_state_get_ticktimer(game_state *gs);
int game_state_serialize(game_state *gs, serial *ser);
int game_state_unserialize(game_state *gs, serial *ser, int rtt);

void _setup_keyboard(game_state *gs, int player_id);
void _setup_ai(game_state *gs, int player_id);
void _setup_joystick(game_state *gs, int player_id, int joystick);
void reconfigure_controller(game_state *gs);

int game_state_rewind(game_state *gs, int rtt);
void game_state_replay(game_state *gs, int rtt);

int game_state_add_object(game_state *gs, object *obj, int layer);
void game_state_del_object(game_state *gs, object *obj);
void game_state_del_animation(game_state *gs, int anim_id);
void game_state_set_speed(game_state *gs, int speed);
void game_state_get_projectiles(game_state *gs, vector *obj_proj);

#endif // _GAME_STATE_H
