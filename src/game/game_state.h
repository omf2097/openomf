#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "formats/rec_assertion.h"
#include "game/game_state_type.h"
#include "game/utils/serial.h"
#include "utils/random.h"
#include "utils/vector.h"
#include <SDL.h>
#include <stdbool.h>

typedef struct scene_t scene;
typedef struct game_player_t game_player;
typedef struct object_t object;
typedef struct ctrl_event_t ctrl_event;

bool game_state_check_assertion_is_met(rec_assertion *ass, game_state *gs);

void game_state_match_settings_reset(game_state *gs);
void game_state_match_settings_defaults(game_state *gs);
int game_state_create(game_state *gs, engine_init_flags *init_flags);
void game_state_free(game_state **gs);
int game_state_handle_event(game_state *gs, SDL_Event *event);
void game_state_render(game_state *gs);
unsigned game_state_get_framebuffer_options(game_state *gs);
void game_state_palette_transform(game_state *gs);
void game_state_debug(game_state *gs);
void game_state_static_tick(game_state *gs, bool replay);
void game_state_dynamic_tick(game_state *gs, bool replay);
void game_state_tick_controllers(game_state *gs);
unsigned int game_state_get_tick(game_state *gs);
scene *game_state_get_scene(game_state *gs);
unsigned int game_state_is_running(game_state *gs);
unsigned int game_state_is_paused(game_state *gs);
void game_state_set_paused(game_state *gs, unsigned int paused);
void game_state_set_next(game_state *gs, unsigned int next_scene_id);
game_player *game_state_get_player(const game_state *gs, int player_id);
int game_state_num_players(game_state *gs);
void game_state_init_demo(game_state *gs);
int game_state_ms_per_dyntick(game_state *gs);
ticktimer *game_state_get_ticktimer(game_state *gs);
bool game_state_hars_are_alive(game_state *gs);

object *game_state_find_object(game_state *gs, uint32_t object_id);
int game_state_find_objects(game_state *gs, vector *out, bool (*predicate)(const object *obj, void *user_data),
                            void *ud);

// used to play sounds that may be subject to rollback (eg sounds from player.c, HAR and arena)
void game_state_play_sound(game_state *gs, int id, float volume, float panning, int pitch);

int game_state_clone(game_state *src, game_state *dst);
void game_state_clone_free(game_state *gs);

void _setup_keyboard(game_state *gs, int player_id, int control_id);
void _setup_ai(game_state *gs, int player_id);
int _setup_joystick(game_state *gs, int player_id, const char *joyname, int offset);
void reconfigure_controller(game_state *gs);

void game_state_menu_poll(game_state *gs, ctrl_event **ev);

int game_state_rewind(game_state *gs, int rtt);
void game_state_replay(game_state *gs, int rtt);

void game_state_slowdown(game_state *gs, int ticks, int rate);
void game_state_hit_pause(game_state *gs);

void game_state_set_speed(game_state *gs, int speed);
unsigned int game_state_get_speed(game_state *gs);

int game_state_add_object(game_state *gs, object *obj, int layer, int singleton, int persistent);
void game_state_del_object(game_state *gs, object *obj);
void game_state_del_animation(game_state *gs, int anim_id);
void game_state_get_projectiles(game_state *gs, vector *obj_proj);
void game_state_clear_objects(game_state *gs, int mask);

bool is_netplay(game_state *gs);
bool is_singleplayer(game_state *gs);
bool is_tournament(game_state *gs);
bool is_demoplay(game_state *gs);
bool is_twoplayer(game_state *gs);
bool is_rec_playback(game_state *gs);
bool is_spectator(game_state *gs);

#endif // GAME_STATE_H
