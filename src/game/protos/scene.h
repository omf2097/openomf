#ifndef SCENE_H
#define SCENE_H

#include "controller/controller.h"
#include "game/common_defines.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/protos/object.h"
#include "game/utils/serial.h"
#include "game/utils/ticktimer.h"
#include "resources/bk.h"
#include "video/surface.h"
#include <SDL.h>

typedef struct scene_t scene;
typedef struct game_player_t game_player;
typedef struct game_state_t game_state;

typedef void (*scene_free_cb)(scene *scene);
typedef int (*scene_event_cb)(scene *scene, SDL_Event *event);
typedef void (*scene_render_cb)(scene *scene);
typedef void (*scene_render_overlay_cb)(scene *scene);
typedef void (*scene_tick_cb)(scene *scene, int paused);
typedef void (*scene_input_poll_cb)(scene *scene);
typedef void (*scene_startup_cb)(scene *scene, int anim_id, int *m_load, int *m_repeat);
typedef int (*scene_anim_prio_override_cb)(scene *scene, int anim_id);
typedef void (*scene_clone_cb)(scene *src, scene *dst);
typedef void (*scene_clone_free_cb)(scene *scene);

struct scene_t {
    game_state *gs;
    int id;
    bk *bk_data;
    af *af_data[2];
    void *userdata;

    scene_free_cb free;
    scene_event_cb event;
    scene_render_cb render;
    scene_render_overlay_cb render_overlay;
    scene_tick_cb static_tick;
    scene_tick_cb dynamic_tick;
    scene_input_poll_cb input_poll;
    scene_startup_cb startup;
    scene_anim_prio_override_cb prio_override;
    scene_clone_cb clone;
    scene_clone_free_cb clone_free;
    ticktimer tick_timer;
};

int scene_create(scene *scene, game_state *gs, int scene_id);
int scene_load_har(scene *scene, int player_id);
void scene_init(scene *scene);
void scene_free(scene *scene);
int scene_event(scene *scene, SDL_Event *event);
void scene_render_overlay(scene *scene);
void scene_render(scene *scene);
void scene_dynamic_tick(scene *scene, int paused);
void scene_static_tick(scene *scene, int paused);
void scene_input_poll(scene *scene);
void scene_startup(scene *scene, int id, int *m_load, int *m_startup);
int scene_anim_prio_override(scene *scene, int anim_id);

int scene_serialize(scene *scene, serial *ser);
int scene_unserialize(scene *scene, serial *ser);

int scene_clone(scene *src, scene *dst);
int scene_clone_free(scene *sc);

void scene_set_userdata(scene *scene, void *userdata);
void *scene_get_userdata(scene *scene);

void scene_set_free_cb(scene *scene, scene_free_cb cbfunc);
void scene_set_event_cb(scene *scene, scene_event_cb cbfunc);
void scene_set_render_cb(scene *scene, scene_render_cb cbfunc);
void scene_set_render_overlay_cb(scene *scene, scene_render_overlay_cb cbfunc);
void scene_set_dynamic_tick_cb(scene *scene, scene_tick_cb cbfunc);
void scene_set_static_tick_cb(scene *scene, scene_tick_cb cbfunc);
void scene_set_input_poll_cb(scene *scene, scene_input_poll_cb cbfunc);
void scene_set_startup_cb(scene *scene, scene_startup_cb cbfunc);
void scene_set_anim_prio_override_cb(scene *scene, scene_anim_prio_override_cb cbfunc);
void cb_scene_spawn_object(object *parent, int id, vec2i pos, vec2f vel, uint8_t flags, int s, int g, void *userdata);
void cb_scene_destroy_object(object *parent, int id, void *userdata);

#endif // SCENE_H
